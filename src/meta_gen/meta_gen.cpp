#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/Tooling.h"

#include "clang/AST/RecordLayout.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

#include "clang/Tooling/CommonOptionsParser.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"

#include "meta_type_handler.h"
#include "path_util.h"
#include "reflection.h"
#include "type_info.h"

#include <regex>

#pragma optimize("", off)

static llvm::cl::OptionCategory MyToolCategory("dumb_meta options");

using namespace clang::tooling;
using namespace clang;
using namespace clang::ast_matchers;
using namespace llvm;

static cl::extrahelp MoreHelp("\n-src_root <source path>"
                              "\n-single_meta_storage_path [meta file path]");
static cl::opt<std::string> project_src_root("src_root",
                                             cl::cat(MyToolCategory));
static cl::opt<std::string> meta_storage_path("single_meta_storage_path",
                                              cl::cat(MyToolCategory));

class ClassMatcher : public MatchFinder::MatchCallback {
private:
  // recursive visitor?

  std::vector<std::unique_ptr<TypeInterface::Type>> &types_;

  void init_type(TypeInterface::Type &type, const clang::ASTContext *ctx,
                 const clang::DeclaratorDecl *var) {

    clang::LangOptions langopts{};
    // langopts.CPlusPlus = true;

    clang::PrintingPolicy policy(langopts);
    policy.SuppressTagKeyword = true;

    // strip off typedefs
    // auto ast_type = field->getType().getDesugaredType(*ctx);
    auto qual_type = var->getType().getCanonicalType();
    auto qual_type_no_qualifier = qual_type.getUnqualifiedType();
    auto ast_type = qual_type.getTypePtr();

    type.qualified_name = qual_type_no_qualifier.getAsString(policy);

    policy.SuppressScope = true;
    type.name = qual_type_no_qualifier.getAsString(policy);
    type.size = ctx->getTypeSize(qual_type_no_qualifier) / CHAR_BIT;
    type.alignment = ctx->getTypeAlign(qual_type_no_qualifier) / CHAR_BIT;

    if (const clang::AnnotateAttr *attr = find_attr(var)) {
      std::string annotation = attr->getAnnotation();
      parse_attribute(annotation, type.attrs);
    }
  }

  bool dump_type(const clang::ASTContext *ctx,
                 const clang::CXXMethodDecl *method,
                 TypeInterface::Method &ret) {

    // I don't count on default ctor/operators
    if (method->isImplicit() || method->isCopyAssignmentOperator() ||
        clang::dyn_cast<CXXDestructorDecl>(method) != nullptr)
      return false;

    ret.name = method->getNameAsString();
    ret.is_static = method->isStatic();
    ret.is_ctor = clang::dyn_cast<CXXConstructorDecl>(method) != nullptr;
    // TODO: handle implicit ctor

    if (const clang::AnnotateAttr *attr = find_attr(method)) {
      std::string annotation = attr->getAnnotation();
      parse_attribute(annotation, ret.attrs);
    }

    for (auto param = method->param_begin(); param != method->param_end();
         ++param) {
      TypeInterface::Type type{};
      init_type(type, ctx, *param);
      ret.params.push_back(type);
    }

    return true;
  }

  TypeInterface::Field dump_type(const clang::ASTContext *ctx,
                                 const clang::DeclaratorDecl *var) {
    TypeInterface::Field field_type{};

    TypeInterface::Type *tc = new TypeInterface::Type();
    init_type(*tc, ctx, var);

    field_type.acc = static_cast<TypeInterface::AccessLevel>(var->getAccess());
    field_type.name = var->getNameAsString();
    field_type.type.reset(tc);
    field_type.is_readonly = var->getType().isConstQualified();

    // field_out.is_const = qual_type.isConstQualified();
    // field_out.is_volatile = qual_type.isVolatileQualified();
    // field_out.is_reference = ast_type->isReferenceType();

    return field_type;
  }

  std::unique_ptr<TypeInterface::Type>
  dump_type(const clang::ASTContext *ctx, const clang::CXXRecordDecl *cls) {
    auto ast_type_cls = cls->getTypeForDecl();
    auto ast_qual_type_cls = ast_type_cls->getCanonicalTypeInternal();

    TypeInterface::TypeContainer *tc = new TypeInterface::TypeContainer();
    tc->name = cls->getNameAsString();
    tc->qualified_name = cls->getQualifiedNameAsString();
    tc->size = ctx->getTypeSize(ast_type_cls) / CHAR_BIT;
    tc->alignment = ctx->getTypeAlign(ast_type_cls) / CHAR_BIT;

    auto loc = ctx->getFullLoc(cls->getBeginLoc());
    if (loc.isValid()) {
      auto path = loc.getFileLoc().getFileEntry()->tryGetRealPathName();
      tc->source_loc = path;
    }

    for (auto method = cls->method_begin(); method != cls->method_end();
         ++method) {
      if (!check_attr(*method))
        continue;

      TypeInterface::Method m{};
      if (dump_type(ctx, *method, m)) {

        // TODO: only process public functions now
        if (m.acc == TypeInterface::E_public) {
          tc->methods.push_back(m);
        }
      }
    }

    const clang::ASTRecordLayout &layout = ctx->getASTRecordLayout(cls);

    for (auto field = cls->field_begin(); field != cls->field_end(); ++field) {
      if (!check_attr(*field))
        continue;
      bool std = field->isInStdNamespace();
      bool canonical = field->isCanonicalDecl();

      TypeInterface::Field field_type = dump_type(ctx, *field);
      field_type.offset = layout.getFieldOffset(field->getFieldIndex());

      // TODO: only process public member now
      if (field_type.acc == TypeInterface::E_public) {
        tc->fields.push_back(std::move(field_type));
      }
    }

    return std::unique_ptr<TypeInterface::Type>(tc);
  }

  clang::AnnotateAttr *find_attr(const clang::Decl *stmt) {
    for (auto attr : stmt->getAttrs()) {
      auto annotate = dyn_cast<AnnotateAttr>(attr);
      if (annotate != nullptr &&
          annotate->getAnnotation().startswith(_META_KEYWORD ","))
        return annotate;
    }
    return nullptr;
  };

  // only for exclude annotate
  bool check_attr(const clang::Decl *var) {
    for (auto attr : var->getAttrs()) {
      auto annotate = dyn_cast<AnnotateAttr>(attr);
      if (annotate != nullptr &&
          annotate->getAnnotation().startswith(_META_EXCLUDE_KEYWORD))
        return false;
    }
    return true;
  };

  void parse_attribute(const std::string &attr,
                       std::vector<TypeInterface::Attribute> &attrs) {
    auto real_attr = attr.substr(sizeof(_META_KEYWORD));

    // Range(10, 10), EditorVisible()
    const std::regex re{R"-((\w+)\(([^\)]*)\))-"};

    std::smatch m;
    while (std::regex_search(real_attr, m, re)) {
      if (m.size() == 3) {
        auto obj = m[1].str();
        auto args = m[2].str();

        attrs.push_back({obj, args});

        real_attr = m.suffix();
      }
    }
  }

public:
  ClassMatcher(std::vector<std::unique_ptr<TypeInterface::Type>> &types)
      : types_{types} {}

  void run(const MatchFinder::MatchResult &Result) override {
    auto ast_context = Result.Context;

    // TODO: meta data
    if (const clang::CXXRecordDecl *cls =
            Result.Nodes.getNodeAs<clang::CXXRecordDecl>("cls")) {
      if (const clang::AnnotateAttr *attr = find_attr(cls)) {
        auto t = dump_type(Result.Context, cls);

        std::string annotation = attr->getAnnotation();

        parse_attribute(annotation, t->attrs);

        types_.push_back(std::move(t));
      }
    } else if (const clang::VarDecl *var =
                   Result.Nodes.getNodeAs<clang::VarDecl>("var")) {
      if (check_attr(var)) {
        // static member
        static_cast<TypeInterface::TypeContainer *>(types_.back().get())
            ->fields.push_back(dump_type(Result.Context, var));
      }
    }
  }

  void onStartOfTranslationUnit() override {}

  void onEndOfTranslationUnit() override {}
};

int main(int argc, const char *argv[]) {
  llvm::EnablePrettyStackTrace();
  llvm::sys::PrintStackTraceOnErrorSignal(argv[0]);

  CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);

  // use my own source list
  std::vector<std::string> source_list{};

  if (!project_src_root.empty()) {
    list_dir(source_list, project_src_root, "*.h", true);
  } else {
    source_list.insert(begin(source_list),
                       begin(OptionsParser.getSourcePathList()),
                       end(OptionsParser.getSourcePathList()));
  }

  ClangTool tool(OptionsParser.getCompilations(), source_list);

  std::vector<std::unique_ptr<TypeInterface::Type>> types;

  ClassMatcher matcher(types);
  MatchFinder finder;

  // doc: http://clang.llvm.org/docs/LibASTMatchersReference.html
  // match cxx record which has attributes and bind it to "cls" and bind it's
  // variables to "var"
  auto cls_matcher_expr =
      cxxRecordDecl(hasAttr(clang::attr::Annotate)).bind("cls");

  // static member
  auto var_matcher_expr = cxxRecordDecl(hasAttr(clang::attr::Annotate),
                                        forEach(varDecl().bind("var")));

  finder.addMatcher(cls_matcher_expr, &matcher);
  finder.addMatcher(var_matcher_expr, &matcher);

  int result = tool.run(newFrontendActionFactory(&finder).get());

  if (result == 0) {
    bool processed = process_type(types, meta_storage_path);

    if (!processed)
      return -1;
  }

  return result;
}

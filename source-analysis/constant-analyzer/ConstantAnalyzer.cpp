//------------------------------------------------------------------------------
// Tooling sample. Demonstrates:
//
// * How to write a simple source tool using libTooling.
// * How to use RecursiveASTVisitor to find interesting AST nodes.
// * How to use the Rewriter API to rewrite the source code.
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//------------------------------------------------------------------------------
#include <sstream>
#include <string>
#include <iostream>
#include <list>



#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclarationName.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ParentMapContext.h"
#include "clang/AST/RecursiveASTVisitor.h"


#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Frontend/FrontendPluginRegistry.h"

#include "llvm/Support/raw_ostream.h"


using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;

static llvm::cl::OptionCategory ToolingSampleCategory("Tooling Sample");

// By implementing RecursiveASTVisitor, we can specify which AST nodes
// we're interested in by overriding relevant methods.
class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor> {

private:
  clang::ASTContext *Context;
  clang::DiagnosticsEngine *Diagnostics;
  const clang::SourceManager &SourceManager = Context->getSourceManager();

  NamedDecl* current_func;
  std::string current_func_name;

  enum constant_type {
    INT_LIT = 0,
    INT_GLOBAL = 1,
    INT_ENUM = 2,
    STRING_LIT = 3
  };

  struct EnumValue
  {
  	std::string enum_name = "";
  	int value;
    EnumDecl *enum_decl;

    SourceLocation source_location;

 	  EnumValue(std::string name, int value, EnumDecl *enum_decl, SourceLocation source_location)
 	  {
 		   this->enum_name = name;
 		   this->value = value;
       this->enum_decl = enum_decl;
       this->source_location = source_location;
 	  }
  };

  struct EnumContainer
  {
  	std::string container_name = "";
  	std::list<EnumValue*> enum_list;

  	EnumContainer(std::string name)
 	  {
 		   this->container_name = name;
 	  }
  };

  struct ConstIntegerLit
  {
  	int value;
    std::string function_name = "";

    constant_type const_type;

    // Extra meta data for enum
    std::string enum_name = ""; // if applicable...
    std::string decl_source_line_info = ""; // if applicable...

    std::string source_line_info = "";


    ConstIntegerLit(int value, std::string function_name, enum constant_type const_type)
    {
      this->value = value;
      this->function_name = function_name;
      this->const_type = const_type;
    }

  };

  struct ConstStringLit
  {
  	std::string value;
    std::string function_name = "";

    constant_type const_type;

    std::string source_line_info = "";



  	ConstStringLit(std::string value, std::string function_name, enum constant_type const_type)
  	{
  		this->value = value;
      this->function_name = function_name;
      this->const_type = const_type;
  	}
  };


  // Container that holds all enum definitions, their use will be determined by visited integer literals
  std::list<EnumContainer*> container_list;

  // Lists to hold information regarding every constant value used in the program
  std::list<ConstIntegerLit*> int_lit_list;
  std::list<ConstStringLit*> string_lit_list;




public:

  explicit MyASTVisitor(clang::ASTContext *Context, clang::DiagnosticsEngine *Diagnostics) : Context(Context), Diagnostics(Diagnostics) {}



  /*
    VisitNamedDecl visits all named declarations.

    We only care about function declarations as this function is designed
    to keep a pointer to the function that AST nodes we visit correspond to.
  */
  bool VisitNamedDecl(NamedDecl *ND)
  {
    if (ND->isFunctionOrFunctionTemplate())
    {
      current_func_name = ND->getQualifiedNameAsString();
      current_func = ND;
      //llvm::outs() << "Found " << ND->getQualifiedNameAsString() << " at "
      //             << getDeclLocation(ND->getBeginLoc()) << "\n";
    }

    return true;
  }


  /*
    VisitEnumDecl visits each ast node responsible for declaring a new enumeration
    value. There will be a value attached to each declaration.

    This function is primarily designed to generate a table of
    enumeration containers and a list of their corresponding enum entities.

    This table can be looked up later when analyzing enum expressions
    for the use of each of these enum values.

  */
  bool VisitEnumDecl(EnumDecl *ED)
  {
  	//llvm::StringRef ss;
  	//std::string id = ED->getDefinition()->getIdentifier()->getName().str();
  	//std::string edID = ss.str();
    SourceLocation source_location = ED->getLocation();

  	EnumContainer* ec = new EnumContainer(ED->getNameAsString());

  	for (auto it = ED->enumerator_begin(); it != ED->enumerator_end(); it++){
  		EnumValue* new_enum_val = new EnumValue(it->getNameAsString(), it->getInitVal().getSExtValue(), ED, source_location);

  		ec->enum_list.push_back(new_enum_val);

         	//std::cout <<it->getNameAsString()<<" "<<it->getInitVal().getSExtValue()<<std::endl;
      }

      container_list.push_back(ec);

    	return true;
  }


  bool VisitVarDecl(clang::VarDecl *D) {
    auto &SM = Context->getSourceManager();
    auto &LO = Context->getLangOpts();
    auto DeclarationType = D->getTypeSourceInfo()->getTypeLoc();




      /*const clang::SourceManager &SM = Context->getSourceManager();
      if (D->hasGlobalStorage() && !D->getType().isConstQualified()) {
          clang::FullSourceLoc loc = Context->getFullLoc(D->getBeginLoc());
          if (!SM.isInSystemHeader(loc)) {
                clang::DiagnosticsEngine &D = *Diagnostics;
                unsigned int id = D.getCustomDiagID(clang::DiagnosticsEngine::Warning, "global variable");
                D.Report(loc, id);

          }
      }
      */


      return true;
  }
















  /*
      SourceManager.getFilename(Loc).str() << ":"
      << SourceManager.getSpellingLineNumber(Loc) << ":"
      << SourceManager.getSpellingColumnNumber(Loc);
  */


  bool VisitIntegerLiteral(IntegerLiteral *IL)
  {

    bool malformed_source_file = false;

  	SourceLocation source_location = IL->getLocation();

    if (SourceManager.getFilename(source_location) == "") {
      malformed_source_file = true;
      //std::cout << "Line:" << SourceManager.getSpellingLineNumber(source_location) << ":" << std::endl;
      //std::cout << "File:" << SourceManager.getFilename(current_func->getLocation()).str() << ":" << std::endl;
    }

    llvm::APInt ap_int = IL->getValue();

    unsigned num_words = ap_int.getNumWords();
    if (num_words == 1)
    {
      int64_t int_value;

      const clang::Stmt* ST;
      const clang::Expr* EX;
      const clang::Decl* DC;

      const clang::DynTypedNodeList &parents = Context->getParents(*IL);

      for (int k = 0; k < parents.size(); k++){
         ST = parents[k].get<Stmt>();
         EX = parents[k].get<Expr>();
         DC = parents[k].get<Decl>();

         if (!ST && !EX && !DC)
         {
           break;
         }

         if (EX){

           /*
              Check if parent is a unaryoperator.
              If this is the case, check if the operator is a 'not'; representing negation as negative integers are not stored as negative.
           */
           if (const UnaryOperator *UO = dyn_cast<UnaryOperator>(EX)){
             if (UO->getOpcode() != clang::UnaryOperator::Opcode::UO_Not) {
               int_value = ap_int.getRawData()[0];
               int_value *= -1;

               ConstIntegerLit *temp_int = new ConstIntegerLit(int_value, current_func_name, INT_LIT);

               const clang::DynTypedNodeList &real_filename_parent = getRealParentFilename(parents, 0);

               // Ignore these cases
               if (real_filename_parent.size() == 0){
                // SourceLocation sl = current_func->getLocation();
                // std::ostringstream oss;
                // oss << SourceManager.getFilename(current_func->getLocation()).str() << ":"
                //     << SourceManager.getSpellingLineNumber(source_location) << ":"
                //     << SourceManager.getSpellingColumnNumber(source_location);

                 //temp_int->source_line_info = oss.str();
                 //std::cout << "Int" << ":" << temp_int->value << ":" << temp_int->source_line_info << std::endl;
                 return true;
               } else {
                 SourceLocation sl = real_filename_parent[0].getSourceRange().getBegin();
                 temp_int->source_line_info = getCompleteSourceLocation(sl).str();
                 //std::cout << "Int" << ":" << temp_int->value << ":" << temp_int->source_line_info << std::endl;
               }

               int_lit_list.push_back(temp_int);

               return true;
             }
           }

           const clang::Expr* CEX_EX;

           /*
              Declarations of enumeration values will look like the following
              EnumConstantDecl->ImplicitCastExpr->ConstantExpr->IntegerLiteral

           */
           if (const ConstantExpr *CEX = dyn_cast<ConstantExpr>(EX)){

             // Get parent of IntegerLiteral, if enum next level should be
             const clang::DynTypedNodeList &const_parents = Context->getParents(*CEX);
             for (int k = 0; k < const_parents.size(); k++){
               CEX_EX = const_parents[k].get<Expr>();
               if (CEX_EX)
               {
                 // We actually want to ignore these cases.
                 //ConstIntegerLit *temp_int = new ConstIntegerLit(int_value, current_func_name, INT_ENUM, source_location);
                 //int_lit_list.push_back(temp_int);

                 //return true;
               }
             }
           }
         }
      }

      int_value = ap_int.getRawData()[0];

      /*
      // Ad-hoc check for globals. Note this can be improved as it could miss some globals defined in unorthadox orders
      if (current_func_name == "")
      {
        ConstIntegerLit *temp_int = new ConstIntegerLit(int_value, current_func_name, INT_GLOBAL);
        temp_int->source_line_info = getCompleteSourceLocation(source_location).str();

        int_lit_list.push_back(temp_int);
        return true;
      }
      */

      // Otherwise gather the value of the integer literal and create a element in the list for it
      ConstIntegerLit *temp_int = new ConstIntegerLit(int_value, current_func_name, INT_LIT);
      if (parents.size() == 0){
        return true;
      } else {
        SourceLocation sl = parents[0].getSourceRange().getBegin();

        if (SourceManager.getFilename(sl) == "") {
          /*
          std::ostringstream oss;
          oss << SourceManager.getFilename(current_func->getLocation()).str() << ":"
              << SourceManager.getSpellingLineNumber(source_location) << ":"
              << SourceManager.getSpellingColumnNumber(source_location);

          temp_int->source_line_info = oss.str();
          //std::cout << "Int" << ":" << temp_int->value << ":" << temp_int->source_line_info << std::endl;
          */
          return true;
        }
        else {
          temp_int->source_line_info = getCompleteSourceLocation(sl).str();
          //std::cout << "Int" << ":" << temp_int->value << ":" << temp_int->source_line_info << std::endl;
        }
      }
      int_lit_list.push_back(temp_int);
    }

    return true;
  }



  bool VisitStringLiteral(StringLiteral *SL)
  {

    bool malformed_source_file = false;
  	SourceLocation source_location = SL->getBeginLoc();

    if (SourceManager.getFilename(source_location) == "") {
      malformed_source_file = true;
      //std::cout << "Line:" << SourceManager.getSpellingLineNumber(source_location) << ":" << std::endl;
      //std::cout << "File:" << SourceManager.getFilename(current_func->getLocation()).str() << ":" << std::endl;
    }

    std::string string_text = SL->getString().str();




    // BEGIN FIND ASM STMTS
    const clang::Stmt* ST;
    const clang::Expr* EX;
    const clang::Decl* DC;

    const clang::DynTypedNodeList &parents = Context->getParents(*SL);

    for (int k = 0; k < parents.size(); k++){
       ST = parents[k].get<Stmt>();
       EX = parents[k].get<Expr>();
       DC = parents[k].get<Decl>();

       if (!ST && !EX && !DC)
       {
         break;
       }

       if (EX){
         const clang::Stmt* ASM_ST;
         if (const ImplicitCastExpr *ICE = dyn_cast<ImplicitCastExpr>(EX)){

           const clang::DynTypedNodeList &const_parents = Context->getParents(*ICE);
           for (int k = 0; k < const_parents.size(); k++){
             ASM_ST = const_parents[k].get<Stmt>();
             if (const GCCAsmStmt *gcc_stmt = dyn_cast<GCCAsmStmt>(ASM_ST))
             {

               //std::cout << "ASM:" << string_text << std::endl;
               return true;
             }
           }
         }
       }
    }
    // END FIND ASM STMTS


    ConstStringLit *temp_string = new ConstStringLit(string_text, current_func_name, STRING_LIT);


    std::ostringstream oss;

    if (malformed_source_file)
    {
      oss << SourceManager.getFilename(current_func->getLocation()).str() << ":"
          << SourceManager.getSpellingLineNumber(source_location) << ":"
          << SourceManager.getSpellingColumnNumber(source_location);
    } else {
      oss << SourceManager.getFilename(source_location).str() << ":"
          << SourceManager.getSpellingLineNumber(source_location) << ":"
          << SourceManager.getSpellingColumnNumber(source_location);
    }

    temp_string->source_line_info = oss.str();

  	string_lit_list.push_back(temp_string);

    return true;
  }

  bool VisitDeclRefExpr(DeclRefExpr *DRE){
    SourceLocation source_location = DRE->getLocation();
    std::string decl_name = DRE->getDecl()->getNameAsString();
    EnumValue* found_enum = getEnumFromContainerList(decl_name, getContainerList());

    if (found_enum != nullptr) {
      ConstIntegerLit *temp_int = new ConstIntegerLit(found_enum->value, current_func_name, INT_ENUM);

      if (SourceManager.getFilename(source_location) == "") {

        return true;

      }
      else {
        temp_int->source_line_info = getCompleteSourceLocation(source_location).str();
        //std::cout << "Int" << ":" << temp_int->value << ":" << temp_int->source_line_info << std::endl;
      }


      //temp_int->source_line_info = getCompleteSourceLocation(source_location).str();
      temp_int->enum_name = found_enum->enum_name;
      temp_int->decl_source_line_info = getCompleteSourceLocation(found_enum->enum_decl->getLocation()).str();
      //std::cout << "enum" << ":" << temp_int->value << ":" << temp_int->source_line_info << " <- " << getCompleteSourceLocation(found_enum->enum_decl->getLocation()).str() << std::endl;
      int_lit_list.push_back(temp_int);
    }

    return true;
  }

  std::list<EnumContainer*> getContainerList()
  {
  	return container_list;
  }

  std::list<ConstStringLit*> getStringList() {
    return string_lit_list;
  }

  std::list<ConstIntegerLit*> getIntList() {
    return int_lit_list;
  }


  EnumValue* getEnumFromContainerList(std::string test_name, std::list<EnumContainer*> container_list){
    for (auto temp_cont : container_list){
      for (auto temp_enum : temp_cont->enum_list){
        if (test_name == temp_enum->enum_name){
          return temp_enum;
        }
      }
    }

    return nullptr;
  }

  int countEnumDecls(std::list<EnumContainer*> container_list) {
    int count = 0;

    for (auto temp_cont : container_list) {
      count += temp_cont->enum_list.size();
    }
    return count;
  }

  std::ostringstream getCompleteSourceLocation(SourceLocation source_location) {
    std::ostringstream oss;

    oss << SourceManager.getFilename(source_location).str() << ":"
        << SourceManager.getSpellingLineNumber(source_location) << ":"
        << SourceManager.getSpellingColumnNumber(source_location);

    return oss;
  }

  clang::DynTypedNodeList getRealParentFilename(const clang::DynTypedNodeList &const_parents, int count){

    for (int k = 0; k < const_parents.size(); k++){

       SourceLocation sl = const_parents[k].getSourceRange().getBegin();

       if (SourceManager.getFilename(sl) == "")
       {
         return getRealParentFilename(Context->getParents(const_parents[k]), count++);
       } else {
         //std::cout << "Found filename in: " << count << " iterations" << std::endl;
         return const_parents;
       }
    }
    return const_parents;
  }


};

// Implementation of the ASTConsumer interface for reading an AST produced
// by the Clang parser.
class MyASTConsumer : public ASTConsumer {
public:

  explicit MyASTConsumer(clang::ASTContext *Context, clang::DiagnosticsEngine *Diagnostics) : Visitor(Context, Diagnostics) {}


  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());

    std::cout << "Gathered " << Visitor.countEnumDecls(Visitor.getContainerList()) << " enum declarations across " << Visitor.getContainerList().size() << " enum containers." << std::endl;
    std::cout << "Constant Strings: " << Visitor.getStringList().size() << std::endl;
    std::cout << "Constant Integers: " << Visitor.getIntList().size() << std::endl;


    for (auto temp_int : Visitor.getIntList()){

      if (temp_int->const_type == 1)
      {
        std::cout << ":global" << ":" << temp_int->value << ":" << temp_int->source_line_info << std::endl;
      }
      else if (temp_int->const_type == 2) {
        std::cout << temp_int->function_name << ":enum" << ":" << temp_int->value << ":" << temp_int->source_line_info << " <- " << temp_int->decl_source_line_info << std::endl;
      }
      else {
        //std::cout << temp_int->function_name << ":literal" << ":" << temp_int->value << ":" << temp_int->source_line_info << std::endl;
      }
    }
    


    for (auto temp_string : Visitor.getStringList()) {
      //std::cout << temp_string->function_name << ":string" << ":\"" << temp_string->value << "\":" << temp_string->source_line_info << std::endl;
    }


  }

private:
  MyASTVisitor Visitor;
};

// For each source file provided to the tool, a new FrontendAction is created.
class MyFrontendAction : public ASTFrontendAction {
public:

  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef) {
    return std::unique_ptr<clang::ASTConsumer>(new MyASTConsumer(&CI.getASTContext(), &CI.getDiagnostics()));
  }


};

int main(int argc, const char **argv) {
  auto ExpectedParser = CommonOptionsParser::create(argc, argv, ToolingSampleCategory);
  if (!ExpectedParser) {
    // Fail gracefully for unsupported options.
    llvm::errs() << ExpectedParser.takeError();
    return 1;
  }
  CommonOptionsParser& OptionsParser = ExpectedParser.get();
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());

  //CommonOptionsParser op = CommonOptionsParser::create(argc, argv, ToolingSampleCategory);
  //ClangTool Tool(op.getCompilations(), op.getSourcePathList());

  // ClangTool::run accepts a FrontendActionFactory, which is then used to
  // create new objects implementing the FrontendAction interface. Here we use
  // the helper newFrontendActionFactory to create a default factory that will
  // return a new MyFrontendAction object every time.
  // To further customize this, we could create our own factory class.
  return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}

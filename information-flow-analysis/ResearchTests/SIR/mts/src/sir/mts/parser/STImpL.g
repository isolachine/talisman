header {
/*
 * Copyright (c) 2003-Present Alex Kinneer. All rights reserved.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

package sir.mts.parser;
}

{
import java.util.List;
import java.util.Map;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.io.InputStream;
import java.io.Reader;

import sir.mts.TestScriptGenerator;
import sir.mts.ScriptGenException;
import sir.mts.parser.ParseException;

import antlr.TokenStreamRecognitionException;
}
class STImpLParser extends Parser;

{
  /** The script generator to which STImpL test case commands will be
      reported. */
  private TestScriptGenerator scriptGen;

  /** Current test case number. */
  private int testID = 0;
  /** Flag indicating whether we have already seen an input file command
      (&apos;-I&apos;) for the current test case. */
  private boolean haveInput = false;
  /** Flag indicating whether we have already seen an output file command
      (&apos;-O&apos;) for the current test case. */
  private boolean haveOutput = false;
  /** Flag indicating whether we have already seen a test driver command
      (&apos;-D&apos;) for the current test case. */
  private boolean haveDriver = false;
  /** Flag indicating whether we have already warned about deprecated
      comment commands (&apos;-A&apos;). */
  private boolean depComWarned = false;

  /** Reference to the lexer, used to issue certain callbacks. */
  private STImpLLexer lexer;

  /** Conditional compilation flag to insert debugging outputs. */
  private static final boolean DEBUG = false;

  /**
   * Creates a new STImpL file parser.
   *
   * @param lexer STImpL lexer to use to read the file.
   * @param scriptGen Test script generator that will receive the STImpL
   * test case commands.
   */
  protected STImpLParser(STImpLLexer lexer, TestScriptGenerator scriptGen) {
      this(lexer);
      this.lexer = lexer;
      this.scriptGen = scriptGen;
      this.lexer.setEscapedQuote(scriptGen.getEscapedQuote());
  }

  /**
   * Parses a STImpL file and generates a test script.
   *
   * @param in Stream from which to read the STImpL file.
   * @param scriptGen Test script generator to be used.
   *
   * @throws ParseException If the STImpL file is not correctly written.
   * @throws ScriptGenException If there is an error while attempting to
   * generate the test script.
   */
  public static void parse(InputStream in, TestScriptGenerator scriptGen)
          throws ParseException, ScriptGenException {
      doParse(new STImpLParser(new STImpLLexer(in), scriptGen));
  }

  /**
   * Parses a STImpL file and generates a test script.
   *
   * @param in Stream from which to read the STImpL file.
   * @param scriptGen Test script generator to be used.
   * @param disableEscapes Flag that controls whether slash-escapes are to
   * be recognized in the STImpL file.
   *
   * @throws ParseException If the STImpL file is not correctly written.
   * @throws ScriptGenException If there is an error while attempting to
   * generate the test script.
   */
  public static void parse(InputStream in, TestScriptGenerator scriptGen,
                           boolean disableEscapes)
          throws ParseException, ScriptGenException {
      doParse(new STImpLParser(new STImpLLexer(in, disableEscapes), scriptGen));
  }

  /**
   * Parses a STImpL file and generates a test script.
   *
   * @param in Reader from which to read the STImpL file.
   * @param scriptGen Test script generator to be used.
   *
   * @throws ParseException If the STImpL file is not correctly written.
   * @throws ScriptGenException If there is an error while attempting to
   * generate the test script.
   */
  public static void parse(Reader in, TestScriptGenerator scriptGen)
          throws ParseException, ScriptGenException {
      doParse(new STImpLParser(new STImpLLexer(in), scriptGen));
  }

  /**
   * Parses a STImpL file and generates a test script.
   *
   * @param in Reader from which to read the STImpL file.
   * @param scriptGen Test script generator to be used.
   * @param disableEscapes Flag that controls whether slash-escapes are to
   * be recognized in the STImpL file.
   *
   * @throws ParseException If the STImpL file is not correctly written.
   * @throws ScriptGenException If there is an error while attempting to
   * generate the test script.
   */
  public static void parse(Reader in, TestScriptGenerator scriptGen,
                           boolean disableEscapes)
          throws ParseException, ScriptGenException {
      doParse(new STImpLParser(new STImpLLexer(in, disableEscapes), scriptGen));
  }

  /**
   * Initiates the actual parsing of a STImpL file.
   *
   * @param parser STImpL parser to be run.
   *
   * @throws ParseException If the STImpL file is not correctly written.
   * @throws ScriptGenException If there is an error while attempting to
   * generate the test script.
   */
  private static void doParse(STImpLParser parser)
          throws ParseException, ScriptGenException {
      try {
          parser.stimple_file();
      }
      catch (RecognitionException e) {
          throw new ParseException("Invalid STImplL file: Line " +
              e.getLine() + ", column " + e.getColumn() + ":\n" +
              "  " + e.getMessage());
      }
      catch (TokenStreamRecognitionException e) {
          throw new ParseException("Invalid STImplL file: Line " +
              e.recog.getLine() + ", column " + e.recog.getColumn() + ":\n" +
              "  " + e.getMessage());
      }
      catch (TokenStreamException e) {
          throw new ParseException("Invalid STImplL file: " + e.getMessage());
      }
  }
}

stimple_file throws ScriptGenException
  { scriptGen.startScript(); }
  : ( assign_var
    |
       {
        haveInput = false;
        haveOutput = false;
        haveDriver = false;
        testID += 1;
        lexer.setTestNumber(testID);
        scriptGen.newTest(testID);
       }
      test_spec
       {
        scriptGen.endTest();
       }
    )+
  { scriptGen.endScript(); }
  ;

// We accept C shell style variable assignment syntax. A special case
// of Bourne shell style variable assignment is accepted to set the
// CLASSPATH variable, which gets mapped to the same variable
// assignment interface call.
assign_var throws ScriptGenException
{ String var, val; }
  : (
      (ASSIGN_VAR var=data val=data NEWLINE
      {
        scriptGen.addVariableAssign(var, val, true);

        if (DEBUG) {
          System.out.println("Assign variable:");
          System.out.println("  " + var + ":=" + val);
        }
      })
    | (SET_CLASSPATH val=data NEWLINE
      {
        scriptGen.addVariableAssign("CLASSPATH", val, true);

        if (DEBUG) {
          System.out.println("Assign variable:");
          System.out.println("  " + var + ":=" + val);
        }
      })
    )
  ;

test_spec throws ScriptGenException
  :
   ( param
   | input
   | output
   | move
   | ext_setup
   | inl_setup
   | ext_finish
   | inl_finish
   | comment
   | driver
   | background
   | deprecated__comment
   )+
   NEWLINE
  ;

param throws ScriptGenException
{
  String param;
  List params = null;
  if (DEBUG) params = new ArrayList();
}
  : ("-P" BEGIN (param=data
      {scriptGen.addParameter(param); if (DEBUG) params.add(param); })* END
  {
    if (DEBUG) {
      System.out.println("Parameter strings:");
      System.out.print("  {");
      for (int i = 0; i < params.size(); i++) {
        System.out.print(" [" + i + "]\"" + params.get(i) +"\"");
      }
      System.out.println(" }");
    }
  })
  ;

// It would be very complex to try to write grammar rules to accept only
// one input file ('-I') command, but at any arbritary position in the test
// case specification. Instead, we just set a flag on the first one and
// then raise a parse error manually if we see it again. This makes the
// STImpL grammar not well defined in some sense, but it's just not worth
// the hassle.
input throws ScriptGenException
{ String file = ""; }
  : (flg:"-I" BEGIN (file=data)? END
  {
    if (haveInput) {
      throw new RecognitionException("Line " + flg.getLine() + ", column " +
        (flg.getColumn() + 1) + ", cannot specify more than one input file " +
        "(\"" + file + "\")");
    }
    haveInput = true;
    scriptGen.addInputFile(file);

    if (DEBUG) {
      System.out.println("Input file: \"" + file + "\"");
    }
  })
  ;

// Similar considerations apply to the '-O' command as to the '-I' command.
output throws ScriptGenException
{ String file; }
  : (flg:"-O" BEGIN file=data END
  {
    if (haveOutput) {
      throw new RecognitionException("Line " + flg.getLine() + ", column " +
        (flg.getColumn() + 1) + ", cannot specify more than one output file " +
        "(\"" + file + "\")");
    }
    haveOutput = true;
    scriptGen.addOutputFile(file);

    if (DEBUG) {
      System.out.println("Output file: \"" + file + "\"");
    }
  })
  ;

move throws ScriptGenException
{ String src, dest; }
  : ("-F" BEGIN src=data BAR dest=data END
  {
    scriptGen.addMoveFile(src, dest);

    if (DEBUG) {
      System.out.println("Move file \"" + src + "\" to \"" +  dest + "\"");
    }
  })
  ;

ext_setup throws ScriptGenException
{ String script, p; List params = new ArrayList(); }
  : ("-S" BEGIN script=data (p=data {params.add(p);})* END
  {
    scriptGen.addExternalSetupScript(script, params);

    if (DEBUG) {
      System.out.println("External setup script: \"" + script + "\"");
      System.out.println("  Parameters:");
      System.out.print("  {");
      for (int i = 0; i < params.size(); i++) {
        System.out.print(" [" + i + "]\"" + params.get(i) +"\"");
      }
      System.out.println(" }");
    }
  })
  ;

// The inline script commands ('-s', '-x') have been enhanced to support
// parameters. It is expected that script generators will inline the
// appropriate commands to assign the given values to the given variable
// names before any content in the inline script, in effect emulating
// parameters to the script.
inl_setup throws ScriptGenException
{ String script, var, val; Map params = new HashMap(); }
  : ("-s" BEGIN script=data (var=data(EQ)val=data {params.put(var, val);})* END
  {
    scriptGen.addInlineSetupScript(script, params);

    if (DEBUG) {
      System.out.println("Inline setup script: \"" + script + "\"");
      Iterator iterator = params.keySet().iterator();
      System.out.println("  Parameters:");
      System.out.print("  {");
      for (int i = 0; i < params.size(); i++) {
        var = (String) iterator.next();
        System.out.print(" [" + i + "]\"" + var +"\"=\"" +
            params.get(var) + "\"");
      }
      System.out.println(" }");
    }
  })
  ;

ext_finish throws ScriptGenException
{ String script, p; List params = new ArrayList(); }
  : ("-X" BEGIN script=data (p=data {params.add(p);})* END
  {
    scriptGen.addExternalFinishScript(script, params);

    if (DEBUG) {
      System.out.println("External finish script: \"" + script + "\"");
      System.out.println("  Parameters:");
      System.out.print("  {");
      for (int i = 0; i < params.size(); i++) {
        System.out.print(" [" + i + "]\"" + params.get(i) +"\"");
      }
      System.out.println(" }");
    }
  })
  ;

inl_finish throws ScriptGenException
{ String script, var, val; Map params = new HashMap(); }
  : ("-x" BEGIN script=data (var=data(EQ)val=data {params.put(var, val);})* END
  {
    scriptGen.addInlineFinishScript(script, params);

    if (DEBUG) {
      System.out.println("Inline finish script: \"" + script + "\"");
      Iterator iterator = params.keySet().iterator();
      System.out.println("  Parameters:");
      System.out.print("  {");
      for (int i = 0; i < params.size(); i++) {
        var = (String) iterator.next();
        System.out.print(" [" + i + "]\"" + var +"\"=\"" +
            params.get(var) + "\"");
      }
      System.out.println(" }");
    }
  })
  ;

comment throws ScriptGenException
{ StringBuffer text = new StringBuffer(); String dataStr; }
  : ("-C" BEGIN
     (dataStr=ws {text.append(dataStr);}
      | dataStr=data {text.append(dataStr);})+
      END
  {
    scriptGen.addComment(text.toString());

    if (DEBUG) {
      System.out.println("Comment: \"" + text.toString() + "\"");
    }
  })
  ;

// Similar considerations apply to the '-D' command as to the '-I' command.
driver throws ScriptGenException
{ String testDriver; }
  : (flg:"-D" BEGIN testDriver=data END
  {
    if (haveDriver) {
      throw new RecognitionException("Line " + flg.getLine() + ", column " +
        (flg.getColumn() + 1) + ", cannot specify more than one test driver " +
        "(\"" + testDriver + "\")");
    }
    haveDriver = true;
    scriptGen.addTestDriver(testDriver);

    if (DEBUG) {
      System.out.println("Test driver: \"" + testDriver + "\"");
    }
  })
  ;

background throws ScriptGenException
  : ("-B"
  {
    scriptGen.setRunInBackground(true);

    if (DEBUG) {
      System.out.println("Run in background");
    }
  })
  ;

// The '-A' comment command was used in a few universe files, but for the
// reason that the '-C' command was intended. So this command has been
// marked as deprecated and will issue a warning the first time it is
// encountered in a STImpL file. Neither '-C' or '-A' command values
// ever made it into the scripts generated by the old mts tools anyway.
deprecated__comment throws ScriptGenException
{ StringBuffer text = new StringBuffer(); String dataStr; }
  : (flg:"-A" BEGIN
     (dataStr=ws {text.append(dataStr);}
      | dataStr=data {text.append(dataStr);})+
      END
  {
    if (!depComWarned) {
      System.err.println("WARN: Line " + flg.getLine() + ", column " +
        flg.getColumn() + ", '-A' type comment is deprecated. It is " +
        "semantically\nidentical to the '-C' type comment, and existing " +
        "versions of mts/javamts\ndisregard it entirely. These comments " +
        "should be converted to '-C' type comments.\n");
      depComWarned = true;
    }

    scriptGen.addComment(text.toString());

    if (DEBUG) {
      System.out.println("Comment (deprecated): \"" + text.toString() + "\"");
    }
  })
  ;

// Helper methods to return lexer tokens such that they can be assigned
// to variables in the grammar rules

protected
data returns [String dataStr = null]
  : (d:DATA)
  { dataStr = d.getText(); }
  ;

// Comments are copied verbatim, so this rule is used to capture whitespace
protected
ws returns [String dataStr = null]
  : (d:WS)
  { dataStr = d.getText(); }
  ;


class STImpLLexer extends Lexer;

// Legal characters in a STImpL file are most of the ascii characters.
options {
  charVocabulary = '\3'..'\377';
  testLiterals = false;
}

{
    /** Flag specifying whether the lexer should not recognize slash-escapes
        (treating them as regular text instead). */
    private boolean disableEscapes = false;
    /** The escape sequence required to safely produce a quote character in
        the target script language. */
    private String escapedQuote = "\"";

    /** Current test number, received from the parser. */
    private int testNum;

    /** Semantic flag to indicate that we are lexing the data value of
        a STImpL command (text between the brackets). */
    private boolean inData = false;
    /** Semantic flag to indicate that we are lexing the data of a
        STImpL move file command (&apos;-F&apos;). */
    private boolean inMove = false;
    /** Semantic flag to indicate that we are lexing the data value of
        a STImpL inline script command (&apos;-s&apos;, &apos;-x&apos;). */
    private boolean inInlineScript = false;
    /** Semantic flag to indicate that we are lexing the data value of
        a STImpL comment command (&apos;-C&apos;). */
    private boolean inComment = false;

    /**
     * Creates a new STImpL file lexer.
     *
     * @param in Stream from which the STImpL file is to be read.
     * @param disableEscapes Flag indicating whether the lexer should
     * ignore slash-style escapes.
     */
    public STImpLLexer(InputStream in, boolean disableEscapes) {
        this(in);
        this.disableEscapes = disableEscapes;
    }

    /**
     * Creates a new STImpL file lexer.
     *
     * @param in Reader from which the STImpL file is to be read.
     * @param disableEscapes Flag indicating whether the lexer should
     * ignore slash-style escapes.
     */
    public STImpLLexer(Reader in, boolean disableEscapes) {
        this(in);
        this.disableEscapes = disableEscapes;
    }

    /**
     * Sets the escape sequence to generate a single quote character
     * (&quot;).
     *
     * @param escQuote The escape sequence required to safely produce a
     * quote character in the target script language.
     */
    public void setEscapedQuote(String escQuote) {
        this.escapedQuote = escQuote;
    }

    /**
     * Callback function that is used by the parser to inform the lexer
     * what the current test number is.
     *
     * <p>The lexer does not have sufficient semantic context to track
     * the current test number without the assistance of the parser.
     * This function enables the lexer to perform inline token substitution
     * of the current test number when it encounters test number macros.</p>
     *
     * @param testNum Current test case number, as reported by the parser.
     */
    public void setTestNumber(int testNum) {
      this.testNum = testNum;
    }

    /**
     * Determines the appropriate value to insert for a macro.
     *
     * @param substToken Name of the macro that was encountered.
     *
     * @return The appropriate substitution text for the given macro.
     *
     * @throws TokenStreamException If the given macro name is not
     * recognized.
     */
    private String getSubstitution(String substToken)
        throws TokenStreamException {
      String ident = substToken.substring(2, substToken.length() - 1);
      if ("TEST".equals(ident)) {
        return String.valueOf(testNum);
      }
      else if ("QUOTE".equals(ident)) {
        return escapedQuote;
      }
      else {
        throw new TokenStreamRecognitionException(
          new RecognitionException("Unrecognized substitution token: \"" +
            substToken + "\"", getFilename(), getLine(),
            getColumn() - substToken.length()));
      }
    }
}

// Note that whitespace tokens are returned to the parser only when
// we are processing comment command ('-C') data, which allows comment
// text to be reproduced verbatim.
WS
  :
   ( '\u0000'..'\u0009'
   | '\u000B'
   | '\u000C'
   | '\u000E'..'\u0020'
   )+
   { if (!(this.inComment && this.inData)) { $setType(Token.SKIP); } }
  ;

// Unfortunately, newlines are significant, as they were used historically
// to delineate test cases.
NEWLINE
  :
   ('\r' '\n')=> "\r\n"
   | '\r'
   | '\n'
   { newline(); this.inData = false; }
  ;

// The old universe file format did not require '[' to be escaped within
// the data section of a command (after a first '[' but before the matching
// ']'). So if the new slash-style escapes are disabled, we'll accept it,
// but in new STImpL files we want to enforce a well-defined escaping
// discipline.
BEGIN
  : {!this.disableEscapes || !this.inData}? '[' { this.inData = true; }
  ;

END
  : ']'
   {
     // Clear flags that change lexing rules for certain characters
     this.inData = false;
     this.inMove = false;
     this.inInlineScript = false;
     this.inComment = false;
   }
  ;

// The '|' character is significant only when used in the move file
// command ('-F'), as it delineates the two file names. In the exceedingly
// unlikely case that it needs to be used in a file name, it can be
// escaped (assuming escapes are enabled, of course). Elsewhere it can
// be used freely without being escaped.
BAR
  : {this.inMove}?
    '|';

// The '=' character is significant only when used in an inline script
// command ('-s', '-x'), as it is used to delineate variable names and
// values when specifying parameters to the script. It can be used if
// escaped. Elsewhere it can be used freely without being escaped.
EQ
  : {this.inInlineScript}?
    '=';

// All of these tokens will be returned as literals for the convenience
// of the parser.
FLAG options { testLiterals = true; }
  : {!this.inData}?
    '-' (
     'P'
   | 'I'
   | 'O'
   | 'F' { this.inMove = true; }
   | 'S'
   | 'X'
   | 's' { this.inInlineScript = true; }
   | 'x' { this.inInlineScript = true; }
   | 'C' { this.inComment = true; }
   | 'D'
   | 'B'
   | 'A' { this.inComment = true; }
   )
  ;

ASSIGN_VAR options { testLiterals = true; }
  : {!this.inData}? "setenv" { this.inData = true; }
  ;

SET_CLASSPATH options { testLiterals = true; }
  : {!this.inData}? "CLASSPATH=" { this.inData = true; }
  ;

// Basically arbitrary data is accepted inside the data segments of a
// STImpL command (between '[' and ']'), which is achieved by disabling
// most of the lexer token recognition rules with the "inData" semantic
// predicate. Certain characters do have to be escaped with slash-style
// character escapes, unless those are disabled, in which case the only
// escape sequence recognized is "\]". When slash-style escapes are
// enabled, unrecognized character escapes are simply reduced to the
// same character, and "\\" has to be used in the STImpL file to
// produce "\" in the output.
DATA
  : {this.inData}?
   ( {this.disableEscapes}? DATA_NO_ESC
   | DATA_ESC
   )
  ;

protected
DATA_ESC
  :
   ( ESC
   | '\u0021'..'\u003C'
   | {!this.inInlineScript}? '='
   | '\u003E'..'\u005A'
   | '\u005E'..'\u007B'
   | {!this.inMove}? '|'
   | '\u007D'..'\u007E'
   | '\u00A0'..'\u00FF'
   )+
  ;

protected
DATA_NO_ESC
  :
   ( ( '\\' ']' ) => ESC { $setType(ESC); }
   | '\\'
   | '\u0021'..'\u003C'
   | {!this.inInlineScript}? '='
   | '\u003E'..'\u005A'
   | '['
   | '\u005E'..'\u007B'
   | {!this.inMove}? '|'
   | '\u007D'..'\u007E'
   | '\u00A0'..'\u00FF'
   )+
  ;

protected
ESC
  :
   '\\'
   ( '['  { $setText("["); }
   | ']'  { $setText("]"); }
   | {this.inMove}? '|' { $setText("|"); }
   | {this.inInlineScript}? '=' { $setText("="); }
   | '\\' { $setText("\\"); }
   | SUBST { String substToken = $getText;
             $setText(getSubstitution(substToken)); }
   | ~('[' | ']' | '|' | '=' | '\\' | '{')
     { String tokenTxt = $getText; $setText(tokenTxt.substring(1)); }
   )
  ;

// Identifies "\{<macro_name>}" style substitution macro tokens.
protected
SUBST
  : '{' (~'}')+ '}'
  ;

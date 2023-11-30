// $ANTLR 2.7.6 (2005-12-22): "STImpL.g" -> "STImpLParser.java"$

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

import antlr.TokenBuffer;
import antlr.TokenStreamException;
import antlr.TokenStreamIOException;
import antlr.ANTLRException;
import antlr.LLkParser;
import antlr.Token;
import antlr.TokenStream;
import antlr.RecognitionException;
import antlr.NoViableAltException;
import antlr.MismatchedTokenException;
import antlr.SemanticException;
import antlr.ParserSharedInputState;
import antlr.collections.impl.BitSet;

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

public class STImpLParser extends antlr.LLkParser       implements STImpLParserTokenTypes
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

protected STImpLParser(TokenBuffer tokenBuf, int k) {
  super(tokenBuf,k);
  tokenNames = _tokenNames;
}

public STImpLParser(TokenBuffer tokenBuf) {
  this(tokenBuf,1);
}

protected STImpLParser(TokenStream lexer, int k) {
  super(lexer,k);
  tokenNames = _tokenNames;
}

public STImpLParser(TokenStream lexer) {
  this(lexer,1);
}

public STImpLParser(ParserSharedInputState state) {
  super(state,1);
  tokenNames = _tokenNames;
}

	public final void stimple_file() throws RecognitionException, TokenStreamException, ScriptGenException {
		
		scriptGen.startScript();
		
		try {      // for error handling
			{
			int _cnt3=0;
			_loop3:
			do {
				switch ( LA(1)) {
				case ASSIGN_VAR:
				case SET_CLASSPATH:
				{
					assign_var();
					break;
				}
				case 7:
				case 10:
				case 11:
				case 12:
				case 14:
				case 15:
				case 17:
				case 18:
				case 19:
				case 20:
				case 21:
				case 22:
				{
					
					haveInput = false;
					haveOutput = false;
					haveDriver = false;
					testID += 1;
					lexer.setTestNumber(testID);
					scriptGen.newTest(testID);
					scriptGen.setRunInBackground(false);
					test_spec();
					
					scriptGen.endTest();
					
					break;
				}
				default:
				{
					if ( _cnt3>=1 ) { break _loop3; } else {throw new NoViableAltException(LT(1), getFilename());}
				}
				}
				_cnt3++;
			} while (true);
			}
			scriptGen.endScript();
		}
		catch (RecognitionException ex) {
			reportError(ex);
			recover(ex,_tokenSet_0);
		}
	}
	
	public final void assign_var() throws RecognitionException, TokenStreamException, ScriptGenException {
		
		String var, val;
		
		try {      // for error handling
			{
			switch ( LA(1)) {
			case ASSIGN_VAR:
			{
				{
				match(ASSIGN_VAR);
				var=data();
				val=data();
				match(NEWLINE);
				
				scriptGen.addVariableAssign(var, val, true);
				
				if (DEBUG) {
				System.out.println("Assign variable:");
				System.out.println("  " + var + ":=" + val);
				}
				
				}
				break;
			}
			case SET_CLASSPATH:
			{
				{
				match(SET_CLASSPATH);
				val=data();
				match(NEWLINE);
				
				scriptGen.addVariableAssign("CLASSPATH", val, true);
				
				if (DEBUG) {
				System.out.println("Assign variable:");
				System.out.println("  CLASSPATH" + ":=" + val);
				}
				
				}
				break;
			}
			default:
			{
				throw new NoViableAltException(LT(1), getFilename());
			}
			}
			}
		}
		catch (RecognitionException ex) {
			reportError(ex);
			recover(ex,_tokenSet_1);
		}
	}
	
	public final void test_spec() throws RecognitionException, TokenStreamException, ScriptGenException {
		
		
		try {      // for error handling
			{
			int _cnt10=0;
			_loop10:
			do {
				switch ( LA(1)) {
				case 7:
				{
					param();
					break;
				}
				case 10:
				{
					input();
					break;
				}
				case 11:
				{
					output();
					break;
				}
				case 12:
				{
					move();
					break;
				}
				case 14:
				{
					ext_setup();
					break;
				}
				case 15:
				{
					inl_setup();
					break;
				}
				case 17:
				{
					ext_finish();
					break;
				}
				case 18:
				{
					inl_finish();
					break;
				}
				case 19:
				{
					comment();
					break;
				}
				case 20:
				{
					driver();
					break;
				}
				case 21:
				{
					background();
					break;
				}
				case 22:
				{
					deprecated__comment();
					break;
				}
				default:
				{
					if ( _cnt10>=1 ) { break _loop10; } else {throw new NoViableAltException(LT(1), getFilename());}
				}
				}
				_cnt10++;
			} while (true);
			}
			match(NEWLINE);
		}
		catch (RecognitionException ex) {
			reportError(ex);
			recover(ex,_tokenSet_1);
		}
	}
	
	protected final String  data() throws RecognitionException, TokenStreamException {
		String dataStr = null;
		
		Token  d = null;
		
		try {      // for error handling
			{
			d = LT(1);
			match(DATA);
			}
			dataStr = d.getText();
		}
		catch (RecognitionException ex) {
			reportError(ex);
			recover(ex,_tokenSet_2);
		}
		return dataStr;
	}
	
	public final void param() throws RecognitionException, TokenStreamException, ScriptGenException {
		
		
		String param;
		List params = null;
		if (DEBUG) params = new ArrayList();
		
		
		try {      // for error handling
			{
			match(7);
			match(BEGIN);
			{
			_loop14:
			do {
				if ((LA(1)==DATA)) {
					param=data();
					scriptGen.addParameter(param); if (DEBUG) params.add(param);
				}
				else {
					break _loop14;
				}
				
			} while (true);
			}
			match(END);
			
			if (DEBUG) {
			System.out.println("Parameter strings:");
			System.out.print("  {");
			for (int i = 0; i < params.size(); i++) {
			System.out.print(" [" + i + "]\"" + params.get(i) +"\"");
			}
			System.out.println(" }");
			}
			
			}
		}
		catch (RecognitionException ex) {
			reportError(ex);
			recover(ex,_tokenSet_3);
		}
	}
	
	public final void input() throws RecognitionException, TokenStreamException, ScriptGenException {
		
		Token  flg = null;
		String file = "";
		
		try {      // for error handling
			{
			flg = LT(1);
			match(10);
			match(BEGIN);
			{
			switch ( LA(1)) {
			case DATA:
			{
				file=data();
				break;
			}
			case END:
			{
				break;
			}
			default:
			{
				throw new NoViableAltException(LT(1), getFilename());
			}
			}
			}
			match(END);
			
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
			
			}
		}
		catch (RecognitionException ex) {
			reportError(ex);
			recover(ex,_tokenSet_3);
		}
	}
	
	public final void output() throws RecognitionException, TokenStreamException, ScriptGenException {
		
		Token  flg = null;
		String file;
		
		try {      // for error handling
			{
			flg = LT(1);
			match(11);
			match(BEGIN);
			file=data();
			match(END);
			
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
			
			}
		}
		catch (RecognitionException ex) {
			reportError(ex);
			recover(ex,_tokenSet_3);
		}
	}
	
	public final void move() throws RecognitionException, TokenStreamException, ScriptGenException {
		
		String src, dest;
		
		try {      // for error handling
			{
			match(12);
			match(BEGIN);
			src=data();
			match(BAR);
			dest=data();
			match(END);
			
			scriptGen.addMoveFile(src, dest);
			
			if (DEBUG) {
			System.out.println("Move file \"" + src + "\" to \"" +  dest + "\"");
			}
			
			}
		}
		catch (RecognitionException ex) {
			reportError(ex);
			recover(ex,_tokenSet_3);
		}
	}
	
	public final void ext_setup() throws RecognitionException, TokenStreamException, ScriptGenException {
		
		String script, p; List params = new ArrayList();
		
		try {      // for error handling
			{
			match(14);
			match(BEGIN);
			script=data();
			{
			_loop25:
			do {
				if ((LA(1)==DATA)) {
					p=data();
					params.add(p);
				}
				else {
					break _loop25;
				}
				
			} while (true);
			}
			match(END);
			
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
			
			}
		}
		catch (RecognitionException ex) {
			reportError(ex);
			recover(ex,_tokenSet_3);
		}
	}
	
	public final void inl_setup() throws RecognitionException, TokenStreamException, ScriptGenException {
		
		String script, var, val; Map params = new HashMap();
		
		try {      // for error handling
			{
			match(15);
			match(BEGIN);
			script=data();
			{
			_loop30:
			do {
				if ((LA(1)==DATA)) {
					var=data();
					{
					match(EQ);
					}
					val=data();
					params.put(var, val);
				}
				else {
					break _loop30;
				}
				
			} while (true);
			}
			match(END);
			
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
			
			}
		}
		catch (RecognitionException ex) {
			reportError(ex);
			recover(ex,_tokenSet_3);
		}
	}
	
	public final void ext_finish() throws RecognitionException, TokenStreamException, ScriptGenException {
		
		String script, p; List params = new ArrayList();
		
		try {      // for error handling
			{
			match(17);
			match(BEGIN);
			script=data();
			{
			_loop34:
			do {
				if ((LA(1)==DATA)) {
					p=data();
					params.add(p);
				}
				else {
					break _loop34;
				}
				
			} while (true);
			}
			match(END);
			
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
			
			}
		}
		catch (RecognitionException ex) {
			reportError(ex);
			recover(ex,_tokenSet_3);
		}
	}
	
	public final void inl_finish() throws RecognitionException, TokenStreamException, ScriptGenException {
		
		String script, var, val; Map params = new HashMap();
		
		try {      // for error handling
			{
			match(18);
			match(BEGIN);
			script=data();
			{
			_loop39:
			do {
				if ((LA(1)==DATA)) {
					var=data();
					{
					match(EQ);
					}
					val=data();
					params.put(var, val);
				}
				else {
					break _loop39;
				}
				
			} while (true);
			}
			match(END);
			
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
			
			}
		}
		catch (RecognitionException ex) {
			reportError(ex);
			recover(ex,_tokenSet_3);
		}
	}
	
	public final void comment() throws RecognitionException, TokenStreamException, ScriptGenException {
		
		StringBuffer text = new StringBuffer(); String dataStr;
		
		try {      // for error handling
			{
			match(19);
			match(BEGIN);
			{
			int _cnt43=0;
			_loop43:
			do {
				switch ( LA(1)) {
				case WS:
				{
					dataStr=ws();
					text.append(dataStr);
					break;
				}
				case DATA:
				{
					dataStr=data();
					text.append(dataStr);
					break;
				}
				default:
				{
					if ( _cnt43>=1 ) { break _loop43; } else {throw new NoViableAltException(LT(1), getFilename());}
				}
				}
				_cnt43++;
			} while (true);
			}
			match(END);
			
			scriptGen.addComment(text.toString());
			
			if (DEBUG) {
			System.out.println("Comment: \"" + text.toString() + "\"");
			}
			
			}
		}
		catch (RecognitionException ex) {
			reportError(ex);
			recover(ex,_tokenSet_3);
		}
	}
	
	public final void driver() throws RecognitionException, TokenStreamException, ScriptGenException {
		
		Token  flg = null;
		String testDriver;
		
		try {      // for error handling
			{
			flg = LT(1);
			match(20);
			match(BEGIN);
			testDriver=data();
			match(END);
			
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
			
			}
		}
		catch (RecognitionException ex) {
			reportError(ex);
			recover(ex,_tokenSet_3);
		}
	}
	
	public final void background() throws RecognitionException, TokenStreamException, ScriptGenException {
		
		
		try {      // for error handling
			{
			match(21);
			
			scriptGen.setRunInBackground(true);
			
			if (DEBUG) {
			System.out.println("Run in background");
			}
			
			}
		}
		catch (RecognitionException ex) {
			reportError(ex);
			recover(ex,_tokenSet_3);
		}
	}
	
	public final void deprecated__comment() throws RecognitionException, TokenStreamException, ScriptGenException {
		
		Token  flg = null;
		StringBuffer text = new StringBuffer(); String dataStr;
		
		try {      // for error handling
			{
			flg = LT(1);
			match(22);
			match(BEGIN);
			{
			int _cnt51=0;
			_loop51:
			do {
				switch ( LA(1)) {
				case WS:
				{
					dataStr=ws();
					text.append(dataStr);
					break;
				}
				case DATA:
				{
					dataStr=data();
					text.append(dataStr);
					break;
				}
				default:
				{
					if ( _cnt51>=1 ) { break _loop51; } else {throw new NoViableAltException(LT(1), getFilename());}
				}
				}
				_cnt51++;
			} while (true);
			}
			match(END);
			
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
			
			}
		}
		catch (RecognitionException ex) {
			reportError(ex);
			recover(ex,_tokenSet_3);
		}
	}
	
	protected final String  ws() throws RecognitionException, TokenStreamException {
		String dataStr = null;
		
		Token  d = null;
		
		try {      // for error handling
			{
			d = LT(1);
			match(WS);
			}
			dataStr = d.getText();
		}
		catch (RecognitionException ex) {
			reportError(ex);
			recover(ex,_tokenSet_4);
		}
		return dataStr;
	}
	
	
	public static final String[] _tokenNames = {
		"<0>",
		"EOF",
		"<2>",
		"NULL_TREE_LOOKAHEAD",
		"ASSIGN_VAR",
		"NEWLINE",
		"SET_CLASSPATH",
		"\"-P\"",
		"BEGIN",
		"END",
		"\"-I\"",
		"\"-O\"",
		"\"-F\"",
		"BAR",
		"\"-S\"",
		"\"-s\"",
		"EQ",
		"\"-X\"",
		"\"-x\"",
		"\"-C\"",
		"\"-D\"",
		"\"-B\"",
		"\"-A\"",
		"DATA",
		"WS",
		"FLAG",
		"DATA_ESC",
		"DATA_NO_ESC",
		"ESC",
		"SUBST"
	};
	
	private static final long[] mk_tokenSet_0() {
		long[] data = { 2L, 0L};
		return data;
	}
	public static final BitSet _tokenSet_0 = new BitSet(mk_tokenSet_0());
	private static final long[] mk_tokenSet_1() {
		long[] data = { 8314066L, 0L};
		return data;
	}
	public static final BitSet _tokenSet_1 = new BitSet(mk_tokenSet_1());
	private static final long[] mk_tokenSet_2() {
		long[] data = { 25240096L, 0L};
		return data;
	}
	public static final BitSet _tokenSet_2 = new BitSet(mk_tokenSet_2());
	private static final long[] mk_tokenSet_3() {
		long[] data = { 8314016L, 0L};
		return data;
	}
	public static final BitSet _tokenSet_3 = new BitSet(mk_tokenSet_3());
	private static final long[] mk_tokenSet_4() {
		long[] data = { 25166336L, 0L};
		return data;
	}
	public static final BitSet _tokenSet_4 = new BitSet(mk_tokenSet_4());
	
	}

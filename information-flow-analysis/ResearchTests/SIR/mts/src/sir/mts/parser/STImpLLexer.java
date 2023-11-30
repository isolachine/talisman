// $ANTLR 2.7.6 (2005-12-22): "STImpL.g" -> "STImpLLexer.java"$

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

import java.io.InputStream;
import antlr.TokenStreamException;
import antlr.TokenStreamIOException;
import antlr.TokenStreamRecognitionException;
import antlr.CharStreamException;
import antlr.CharStreamIOException;
import antlr.ANTLRException;
import java.io.Reader;
import java.util.Hashtable;
import antlr.CharScanner;
import antlr.InputBuffer;
import antlr.ByteBuffer;
import antlr.CharBuffer;
import antlr.Token;
import antlr.CommonToken;
import antlr.RecognitionException;
import antlr.NoViableAltForCharException;
import antlr.MismatchedCharException;
import antlr.TokenStream;
import antlr.ANTLRHashString;
import antlr.LexerSharedInputState;
import antlr.collections.impl.BitSet;
import antlr.SemanticException;

public class STImpLLexer extends antlr.CharScanner implements STImpLParserTokenTypes, TokenStream
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
public STImpLLexer(InputStream in) {
	this(new ByteBuffer(in));
}
public STImpLLexer(Reader in) {
	this(new CharBuffer(in));
}
public STImpLLexer(InputBuffer ib) {
	this(new LexerSharedInputState(ib));
}
public STImpLLexer(LexerSharedInputState state) {
	super(state);
	caseSensitiveLiterals = true;
	setCaseSensitive(true);
	literals = new Hashtable();
	literals.put(new ANTLRHashString("-C", this), new Integer(19));
	literals.put(new ANTLRHashString("-F", this), new Integer(12));
	literals.put(new ANTLRHashString("-x", this), new Integer(18));
	literals.put(new ANTLRHashString("-I", this), new Integer(10));
	literals.put(new ANTLRHashString("-O", this), new Integer(11));
	literals.put(new ANTLRHashString("-X", this), new Integer(17));
	literals.put(new ANTLRHashString("-A", this), new Integer(22));
	literals.put(new ANTLRHashString("-D", this), new Integer(20));
	literals.put(new ANTLRHashString("-s", this), new Integer(15));
	literals.put(new ANTLRHashString("-P", this), new Integer(7));
	literals.put(new ANTLRHashString("-S", this), new Integer(14));
	literals.put(new ANTLRHashString("-B", this), new Integer(21));
}

public Token nextToken() throws TokenStreamException {
	Token theRetToken=null;
tryAgain:
	for (;;) {
		Token _token = null;
		int _ttype = Token.INVALID_TYPE;
		resetText();
		try {   // for char stream error handling
			try {   // for lexical error handling
				switch ( LA(1)) {
				case '\u0000':  case '\u0001':  case '\u0002':  case '\u0003':
				case '\u0004':  case '\u0005':  case '\u0006':  case '\u0007':
				case '\u0008':  case '\t':  case '\u000b':  case '\u000c':
				case '\u000e':  case '\u000f':  case '\u0010':  case '\u0011':
				case '\u0012':  case '\u0013':  case '\u0014':  case '\u0015':
				case '\u0016':  case '\u0017':  case '\u0018':  case '\u0019':
				case '\u001a':  case '\u001b':  case '\u001c':  case '\u001d':
				case '\u001e':  case '\u001f':  case ' ':
				{
					mWS(true);
					theRetToken=_returnToken;
					break;
				}
				case '\n':  case '\r':
				{
					mNEWLINE(true);
					theRetToken=_returnToken;
					break;
				}
				case ']':
				{
					mEND(true);
					theRetToken=_returnToken;
					break;
				}
				default:
					if (((LA(1)=='['))&&(!this.disableEscapes || !this.inData)) {
						mBEGIN(true);
						theRetToken=_returnToken;
					}
					else if (((LA(1)=='|'))&&(this.inMove)) {
						mBAR(true);
						theRetToken=_returnToken;
					}
					else if (((LA(1)=='='))&&(this.inInlineScript)) {
						mEQ(true);
						theRetToken=_returnToken;
					}
					else if (((LA(1)=='-'))&&(!this.inData)) {
						mFLAG(true);
						theRetToken=_returnToken;
					}
					else if (((LA(1)=='s'))&&(!this.inData)) {
						mASSIGN_VAR(true);
						theRetToken=_returnToken;
					}
					else if (((LA(1)=='C'))&&(!this.inData)) {
						mSET_CLASSPATH(true);
						theRetToken=_returnToken;
					}
					else if (((_tokenSet_0.member(LA(1))))&&(this.inData)) {
						mDATA(true);
						theRetToken=_returnToken;
					}
				else {
					if (LA(1)==EOF_CHAR) {uponEOF(); _returnToken = makeToken(Token.EOF_TYPE);}
				else {throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());}
				}
				}
				if ( _returnToken==null ) continue tryAgain; // found SKIP token
				_ttype = _returnToken.getType();
				_returnToken.setType(_ttype);
				return _returnToken;
			}
			catch (RecognitionException e) {
				throw new TokenStreamRecognitionException(e);
			}
		}
		catch (CharStreamException cse) {
			if ( cse instanceof CharStreamIOException ) {
				throw new TokenStreamIOException(((CharStreamIOException)cse).io);
			}
			else {
				throw new TokenStreamException(cse.getMessage());
			}
		}
	}
}

	public final void mWS(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
		int _ttype; Token _token=null; int _begin=text.length();
		_ttype = WS;
		int _saveIndex;
		
		{
		int _cnt58=0;
		_loop58:
		do {
			switch ( LA(1)) {
			case '\u0000':  case '\u0001':  case '\u0002':  case '\u0003':
			case '\u0004':  case '\u0005':  case '\u0006':  case '\u0007':
			case '\u0008':  case '\t':
			{
				matchRange('\u0000','\u0009');
				break;
			}
			case '\u000b':
			{
				match('\u000B');
				break;
			}
			case '\u000c':
			{
				match('\u000C');
				break;
			}
			case '\u000e':  case '\u000f':  case '\u0010':  case '\u0011':
			case '\u0012':  case '\u0013':  case '\u0014':  case '\u0015':
			case '\u0016':  case '\u0017':  case '\u0018':  case '\u0019':
			case '\u001a':  case '\u001b':  case '\u001c':  case '\u001d':
			case '\u001e':  case '\u001f':  case ' ':
			{
				matchRange('\u000E','\u0020');
				break;
			}
			default:
			{
				if ( _cnt58>=1 ) { break _loop58; } else {throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());}
			}
			}
			_cnt58++;
		} while (true);
		}
		if ( inputState.guessing==0 ) {
			if (!(this.inComment && this.inData)) { _ttype = Token.SKIP; }
		}
		if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
			_token = makeToken(_ttype);
			_token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
		}
		_returnToken = _token;
	}
	
	public final void mNEWLINE(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
		int _ttype; Token _token=null; int _begin=text.length();
		_ttype = NEWLINE;
		int _saveIndex;
		
		boolean synPredMatched61 = false;
		if (((LA(1)=='\r'))) {
			int _m61 = mark();
			synPredMatched61 = true;
			inputState.guessing++;
			try {
				{
				match('\r');
				match('\n');
				}
			}
			catch (RecognitionException pe) {
				synPredMatched61 = false;
			}
			rewind(_m61);
inputState.guessing--;
		}
		if ( synPredMatched61 ) {
			match("\r\n");
		}
		else if ((LA(1)=='\r')) {
			match('\r');
		}
		else if ((LA(1)=='\n')) {
			match('\n');
			if ( inputState.guessing==0 ) {
				newline(); this.inData = false;
			}
		}
		else {
			throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());
		}
		
		if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
			_token = makeToken(_ttype);
			_token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
		}
		_returnToken = _token;
	}
	
	public final void mBEGIN(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
		int _ttype; Token _token=null; int _begin=text.length();
		_ttype = BEGIN;
		int _saveIndex;
		
		if (!(!this.disableEscapes || !this.inData))
		  throw new SemanticException("!this.disableEscapes || !this.inData");
		match('[');
		if ( inputState.guessing==0 ) {
			this.inData = true;
		}
		if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
			_token = makeToken(_ttype);
			_token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
		}
		_returnToken = _token;
	}
	
	public final void mEND(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
		int _ttype; Token _token=null; int _begin=text.length();
		_ttype = END;
		int _saveIndex;
		
		match(']');
		if ( inputState.guessing==0 ) {
			
			// Clear flags that change lexing rules for certain characters
			this.inData = false;
			this.inMove = false;
			this.inInlineScript = false;
			this.inComment = false;
			
		}
		if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
			_token = makeToken(_ttype);
			_token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
		}
		_returnToken = _token;
	}
	
	public final void mBAR(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
		int _ttype; Token _token=null; int _begin=text.length();
		_ttype = BAR;
		int _saveIndex;
		
		if (!(this.inMove))
		  throw new SemanticException("this.inMove");
		match('|');
		if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
			_token = makeToken(_ttype);
			_token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
		}
		_returnToken = _token;
	}
	
	public final void mEQ(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
		int _ttype; Token _token=null; int _begin=text.length();
		_ttype = EQ;
		int _saveIndex;
		
		if (!(this.inInlineScript))
		  throw new SemanticException("this.inInlineScript");
		match('=');
		if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
			_token = makeToken(_ttype);
			_token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
		}
		_returnToken = _token;
	}
	
	public final void mFLAG(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
		int _ttype; Token _token=null; int _begin=text.length();
		_ttype = FLAG;
		int _saveIndex;
		
		if (!(!this.inData))
		  throw new SemanticException("!this.inData");
		match('-');
		{
		switch ( LA(1)) {
		case 'P':
		{
			match('P');
			break;
		}
		case 'I':
		{
			match('I');
			break;
		}
		case 'O':
		{
			match('O');
			break;
		}
		case 'F':
		{
			match('F');
			if ( inputState.guessing==0 ) {
				this.inMove = true;
			}
			break;
		}
		case 'S':
		{
			match('S');
			break;
		}
		case 'X':
		{
			match('X');
			break;
		}
		case 's':
		{
			match('s');
			if ( inputState.guessing==0 ) {
				this.inInlineScript = true;
			}
			break;
		}
		case 'x':
		{
			match('x');
			if ( inputState.guessing==0 ) {
				this.inInlineScript = true;
			}
			break;
		}
		case 'C':
		{
			match('C');
			if ( inputState.guessing==0 ) {
				this.inComment = true;
			}
			break;
		}
		case 'D':
		{
			match('D');
			break;
		}
		case 'B':
		{
			match('B');
			break;
		}
		case 'A':
		{
			match('A');
			if ( inputState.guessing==0 ) {
				this.inComment = true;
			}
			break;
		}
		default:
		{
			throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());
		}
		}
		}
		_ttype = testLiteralsTable(_ttype);
		if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
			_token = makeToken(_ttype);
			_token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
		}
		_returnToken = _token;
	}
	
	public final void mASSIGN_VAR(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
		int _ttype; Token _token=null; int _begin=text.length();
		_ttype = ASSIGN_VAR;
		int _saveIndex;
		
		if (!(!this.inData))
		  throw new SemanticException("!this.inData");
		match("setenv");
		if ( inputState.guessing==0 ) {
			this.inData = true;
		}
		_ttype = testLiteralsTable(_ttype);
		if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
			_token = makeToken(_ttype);
			_token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
		}
		_returnToken = _token;
	}
	
	public final void mSET_CLASSPATH(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
		int _ttype; Token _token=null; int _begin=text.length();
		_ttype = SET_CLASSPATH;
		int _saveIndex;
		
		if (!(!this.inData))
		  throw new SemanticException("!this.inData");
		match("CLASSPATH=");
		if ( inputState.guessing==0 ) {
			this.inData = true;
		}
		_ttype = testLiteralsTable(_ttype);
		if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
			_token = makeToken(_ttype);
			_token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
		}
		_returnToken = _token;
	}
	
	public final void mDATA(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
		int _ttype; Token _token=null; int _begin=text.length();
		_ttype = DATA;
		int _saveIndex;
		
		if (!(this.inData))
		  throw new SemanticException("this.inData");
		{
		if (((_tokenSet_0.member(LA(1))))&&(this.disableEscapes)) {
			mDATA_NO_ESC(false);
		}
		else if ((_tokenSet_1.member(LA(1)))) {
			mDATA_ESC(false);
		}
		else {
			throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());
		}
		
		}
		if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
			_token = makeToken(_ttype);
			_token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
		}
		_returnToken = _token;
	}
	
	protected final void mDATA_NO_ESC(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
		int _ttype; Token _token=null; int _begin=text.length();
		_ttype = DATA_NO_ESC;
		int _saveIndex;
		
		{
		int _cnt79=0;
		_loop79:
		do {
			switch ( LA(1)) {
			case '!':  case '"':  case '#':  case '$':
			case '%':  case '&':  case '\'':  case '(':
			case ')':  case '*':  case '+':  case ',':
			case '-':  case '.':  case '/':  case '0':
			case '1':  case '2':  case '3':  case '4':
			case '5':  case '6':  case '7':  case '8':
			case '9':  case ':':  case ';':  case '<':
			{
				matchRange('\u0021','\u003C');
				break;
			}
			case '>':  case '?':  case '@':  case 'A':
			case 'B':  case 'C':  case 'D':  case 'E':
			case 'F':  case 'G':  case 'H':  case 'I':
			case 'J':  case 'K':  case 'L':  case 'M':
			case 'N':  case 'O':  case 'P':  case 'Q':
			case 'R':  case 'S':  case 'T':  case 'U':
			case 'V':  case 'W':  case 'X':  case 'Y':
			case 'Z':
			{
				matchRange('\u003E','\u005A');
				break;
			}
			case '[':
			{
				match('[');
				break;
			}
			case '^':  case '_':  case '`':  case 'a':
			case 'b':  case 'c':  case 'd':  case 'e':
			case 'f':  case 'g':  case 'h':  case 'i':
			case 'j':  case 'k':  case 'l':  case 'm':
			case 'n':  case 'o':  case 'p':  case 'q':
			case 'r':  case 's':  case 't':  case 'u':
			case 'v':  case 'w':  case 'x':  case 'y':
			case 'z':  case '{':
			{
				matchRange('\u005E','\u007B');
				break;
			}
			case '}':  case '~':
			{
				matchRange('\u007D','\u007E');
				break;
			}
			case '\u00a0':  case '\u00a1':  case '\u00a2':  case '\u00a3':
			case '\u00a4':  case '\u00a5':  case '\u00a6':  case '\u00a7':
			case '\u00a8':  case '\u00a9':  case '\u00aa':  case '\u00ab':
			case '\u00ac':  case '\u00ad':  case '\u00ae':  case '\u00af':
			case '\u00b0':  case '\u00b1':  case '\u00b2':  case '\u00b3':
			case '\u00b4':  case '\u00b5':  case '\u00b6':  case '\u00b7':
			case '\u00b8':  case '\u00b9':  case '\u00ba':  case '\u00bb':
			case '\u00bc':  case '\u00bd':  case '\u00be':  case '\u00bf':
			case '\u00c0':  case '\u00c1':  case '\u00c2':  case '\u00c3':
			case '\u00c4':  case '\u00c5':  case '\u00c6':  case '\u00c7':
			case '\u00c8':  case '\u00c9':  case '\u00ca':  case '\u00cb':
			case '\u00cc':  case '\u00cd':  case '\u00ce':  case '\u00cf':
			case '\u00d0':  case '\u00d1':  case '\u00d2':  case '\u00d3':
			case '\u00d4':  case '\u00d5':  case '\u00d6':  case '\u00d7':
			case '\u00d8':  case '\u00d9':  case '\u00da':  case '\u00db':
			case '\u00dc':  case '\u00dd':  case '\u00de':  case '\u00df':
			case '\u00e0':  case '\u00e1':  case '\u00e2':  case '\u00e3':
			case '\u00e4':  case '\u00e5':  case '\u00e6':  case '\u00e7':
			case '\u00e8':  case '\u00e9':  case '\u00ea':  case '\u00eb':
			case '\u00ec':  case '\u00ed':  case '\u00ee':  case '\u00ef':
			case '\u00f0':  case '\u00f1':  case '\u00f2':  case '\u00f3':
			case '\u00f4':  case '\u00f5':  case '\u00f6':  case '\u00f7':
			case '\u00f8':  case '\u00f9':  case '\u00fa':  case '\u00fb':
			case '\u00fc':  case '\u00fd':  case '\u00fe':  case '\u00ff':
			{
				matchRange('\u00A0','\u00FF');
				break;
			}
			default:
				boolean synPredMatched78 = false;
				if (((LA(1)=='\\'))) {
					int _m78 = mark();
					synPredMatched78 = true;
					inputState.guessing++;
					try {
						{
						match('\\');
						match(']');
						}
					}
					catch (RecognitionException pe) {
						synPredMatched78 = false;
					}
					rewind(_m78);
inputState.guessing--;
				}
				if ( synPredMatched78 ) {
					mESC(false);
					if ( inputState.guessing==0 ) {
						_ttype = ESC;
					}
				}
				else if ((LA(1)=='\\')) {
					match('\\');
				}
				else if (((LA(1)=='='))&&(!this.inInlineScript)) {
					match('=');
				}
				else if (((LA(1)=='|'))&&(!this.inMove)) {
					match('|');
				}
			else {
				if ( _cnt79>=1 ) { break _loop79; } else {throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());}
			}
			}
			_cnt79++;
		} while (true);
		}
		if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
			_token = makeToken(_ttype);
			_token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
		}
		_returnToken = _token;
	}
	
	protected final void mDATA_ESC(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
		int _ttype; Token _token=null; int _begin=text.length();
		_ttype = DATA_ESC;
		int _saveIndex;
		
		{
		int _cnt74=0;
		_loop74:
		do {
			switch ( LA(1)) {
			case '\\':
			{
				mESC(false);
				break;
			}
			case '!':  case '"':  case '#':  case '$':
			case '%':  case '&':  case '\'':  case '(':
			case ')':  case '*':  case '+':  case ',':
			case '-':  case '.':  case '/':  case '0':
			case '1':  case '2':  case '3':  case '4':
			case '5':  case '6':  case '7':  case '8':
			case '9':  case ':':  case ';':  case '<':
			{
				matchRange('\u0021','\u003C');
				break;
			}
			case '>':  case '?':  case '@':  case 'A':
			case 'B':  case 'C':  case 'D':  case 'E':
			case 'F':  case 'G':  case 'H':  case 'I':
			case 'J':  case 'K':  case 'L':  case 'M':
			case 'N':  case 'O':  case 'P':  case 'Q':
			case 'R':  case 'S':  case 'T':  case 'U':
			case 'V':  case 'W':  case 'X':  case 'Y':
			case 'Z':
			{
				matchRange('\u003E','\u005A');
				break;
			}
			case '^':  case '_':  case '`':  case 'a':
			case 'b':  case 'c':  case 'd':  case 'e':
			case 'f':  case 'g':  case 'h':  case 'i':
			case 'j':  case 'k':  case 'l':  case 'm':
			case 'n':  case 'o':  case 'p':  case 'q':
			case 'r':  case 's':  case 't':  case 'u':
			case 'v':  case 'w':  case 'x':  case 'y':
			case 'z':  case '{':
			{
				matchRange('\u005E','\u007B');
				break;
			}
			case '}':  case '~':
			{
				matchRange('\u007D','\u007E');
				break;
			}
			case '\u00a0':  case '\u00a1':  case '\u00a2':  case '\u00a3':
			case '\u00a4':  case '\u00a5':  case '\u00a6':  case '\u00a7':
			case '\u00a8':  case '\u00a9':  case '\u00aa':  case '\u00ab':
			case '\u00ac':  case '\u00ad':  case '\u00ae':  case '\u00af':
			case '\u00b0':  case '\u00b1':  case '\u00b2':  case '\u00b3':
			case '\u00b4':  case '\u00b5':  case '\u00b6':  case '\u00b7':
			case '\u00b8':  case '\u00b9':  case '\u00ba':  case '\u00bb':
			case '\u00bc':  case '\u00bd':  case '\u00be':  case '\u00bf':
			case '\u00c0':  case '\u00c1':  case '\u00c2':  case '\u00c3':
			case '\u00c4':  case '\u00c5':  case '\u00c6':  case '\u00c7':
			case '\u00c8':  case '\u00c9':  case '\u00ca':  case '\u00cb':
			case '\u00cc':  case '\u00cd':  case '\u00ce':  case '\u00cf':
			case '\u00d0':  case '\u00d1':  case '\u00d2':  case '\u00d3':
			case '\u00d4':  case '\u00d5':  case '\u00d6':  case '\u00d7':
			case '\u00d8':  case '\u00d9':  case '\u00da':  case '\u00db':
			case '\u00dc':  case '\u00dd':  case '\u00de':  case '\u00df':
			case '\u00e0':  case '\u00e1':  case '\u00e2':  case '\u00e3':
			case '\u00e4':  case '\u00e5':  case '\u00e6':  case '\u00e7':
			case '\u00e8':  case '\u00e9':  case '\u00ea':  case '\u00eb':
			case '\u00ec':  case '\u00ed':  case '\u00ee':  case '\u00ef':
			case '\u00f0':  case '\u00f1':  case '\u00f2':  case '\u00f3':
			case '\u00f4':  case '\u00f5':  case '\u00f6':  case '\u00f7':
			case '\u00f8':  case '\u00f9':  case '\u00fa':  case '\u00fb':
			case '\u00fc':  case '\u00fd':  case '\u00fe':  case '\u00ff':
			{
				matchRange('\u00A0','\u00FF');
				break;
			}
			default:
				if (((LA(1)=='='))&&(!this.inInlineScript)) {
					match('=');
				}
				else if (((LA(1)=='|'))&&(!this.inMove)) {
					match('|');
				}
			else {
				if ( _cnt74>=1 ) { break _loop74; } else {throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());}
			}
			}
			_cnt74++;
		} while (true);
		}
		if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
			_token = makeToken(_ttype);
			_token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
		}
		_returnToken = _token;
	}
	
	protected final void mESC(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
		int _ttype; Token _token=null; int _begin=text.length();
		_ttype = ESC;
		int _saveIndex;
		
		match('\\');
		{
		switch ( LA(1)) {
		case '[':
		{
			match('[');
			if ( inputState.guessing==0 ) {
				text.setLength(_begin); text.append("[");
			}
			break;
		}
		case ']':
		{
			match(']');
			if ( inputState.guessing==0 ) {
				text.setLength(_begin); text.append("]");
			}
			break;
		}
		case '\\':
		{
			match('\\');
			if ( inputState.guessing==0 ) {
				text.setLength(_begin); text.append("\\");
			}
			break;
		}
		case '{':
		{
			mSUBST(false);
			if ( inputState.guessing==0 ) {
				String substToken = new String(text.getBuffer(),_begin,text.length()-_begin);
				text.setLength(_begin); text.append(getSubstitution(substToken));
			}
			break;
		}
		default:
			if (((LA(1)=='|'))&&(this.inMove)) {
				match('|');
				if ( inputState.guessing==0 ) {
					text.setLength(_begin); text.append("|");
				}
			}
			else if (((LA(1)=='='))&&(this.inInlineScript)) {
				match('=');
				if ( inputState.guessing==0 ) {
					text.setLength(_begin); text.append("=");
				}
			}
			else if ((_tokenSet_2.member(LA(1)))) {
				{
				match(_tokenSet_2);
				}
				if ( inputState.guessing==0 ) {
					String tokenTxt = new String(text.getBuffer(),_begin,text.length()-_begin); text.setLength(_begin); text.append(tokenTxt.substring(1));
				}
			}
		else {
			throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());
		}
		}
		}
		if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
			_token = makeToken(_ttype);
			_token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
		}
		_returnToken = _token;
	}
	
	protected final void mSUBST(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
		int _ttype; Token _token=null; int _begin=text.length();
		_ttype = SUBST;
		int _saveIndex;
		
		match('{');
		{
		int _cnt85=0;
		_loop85:
		do {
			if ((_tokenSet_3.member(LA(1)))) {
				matchNot('}');
			}
			else {
				if ( _cnt85>=1 ) { break _loop85; } else {throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine(), getColumn());}
			}
			
			_cnt85++;
		} while (true);
		}
		match('}');
		if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
			_token = makeToken(_ttype);
			_token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
		}
		_returnToken = _token;
	}
	
	
	private static final long[] mk_tokenSet_0() {
		long[] data = new long[12];
		data[0]=-8589934592L;
		data[1]=9223372036317904895L;
		data[2]=-4294967296L;
		data[3]=-1L;
		return data;
	}
	public static final BitSet _tokenSet_0 = new BitSet(mk_tokenSet_0());
	private static final long[] mk_tokenSet_1() {
		long[] data = new long[12];
		data[0]=-8589934592L;
		data[1]=9223372036183687167L;
		data[2]=-4294967296L;
		data[3]=-1L;
		return data;
	}
	public static final BitSet _tokenSet_1 = new BitSet(mk_tokenSet_1());
	private static final long[] mk_tokenSet_2() {
		long[] data = new long[8];
		data[0]=-2305843009213693953L;
		data[1]=-1729382257849794561L;
		for (int i = 2; i<=3; i++) { data[i]=-1L; }
		return data;
	}
	public static final BitSet _tokenSet_2 = new BitSet(mk_tokenSet_2());
	private static final long[] mk_tokenSet_3() {
		long[] data = new long[8];
		data[0]=-1L;
		data[1]=-2305843009213693953L;
		for (int i = 2; i<=3; i++) { data[i]=-1L; }
		return data;
	}
	public static final BitSet _tokenSet_3 = new BitSet(mk_tokenSet_3());
	
	}

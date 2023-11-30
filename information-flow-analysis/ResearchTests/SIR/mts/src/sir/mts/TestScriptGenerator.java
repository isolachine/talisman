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

package sir.mts;

import java.util.List;
import java.util.Map;

/**
 * A test script generator is responsible for receiving the stream of
 * test implementation commands parsed from a STImpL file and producing
 * the corresponding implementation script in a particular target
 * language.
 *
 * @author Alex Kinneer
 * @version 05/01/2006
 */
public interface TestScriptGenerator {
   /**
    * Indicates the start of a new script.
    *
    * <p>This is most useful for inserting content at the start of the
    * script.</p>
    *
    * @throws ScriptGenException If the script generator fails to generate
    * content associated with this event.
    */
    void startScript() throws ScriptGenException;

    /**
     * Indicates the end of the script.
     *
     * <p>This is most useful for inserting content at the end of
     * the script.</p>
     *
     * @return The number of test cases in the generated script.
     *
     * @throws ScriptGenException If the script generator fails to generate
     * content associated with this event.
     */
    int endScript() throws ScriptGenException;

    /**
     * Copies text verbatim to the generated script.
     *
     * <p><em>Note: There is currently no STImpL syntax associated with this
     * event, and it is not called by the parser. It is reserved for
     * future use.</em></p>
     *
     * @param text Text to be copied unchanged to the generated script.
     *
     * @throws ScriptGenException If the script generator fails to copy the
     * specified text to the script.
     */
    void addVerbatim(String text) throws ScriptGenException;

    /**
     * Inserts a statement in the script to assign a value to a named variable.
     *
     * <p>Variables are considered to exist in the global scope. Thus all
     * test cases subsequent to a variable assignment should have access
     * to the variable value.</p>
     *
     * @param var Name of the variable to be assigned.
     * @param val Value to be assigned to the variable.
     * @param unsetFirst boolean Flag indicating whether a statement to
     * explicitly unset the specified variable should be inserted before
     * the assignment statement, if such a capability exists in the
     * target language.
     *
     * @throws ScriptGenException If the script generator is unable to
     * insert a variable assignment statement.
     */
    void addVariableAssign(String var, String val, boolean unsetFirst)
            throws ScriptGenException;

    /**
     * Indicates the start of a new test implementation specification.
     *
     * @param testID The test number of this test in the script.
     *
     * @throws ScriptGenException If the script generator is unable to
     * generate a new test.
     */
    void newTest(int testID) throws ScriptGenException;

    /**
     * Indicates the end of the current test implementation specification.
     *
     * @return The number of tests added to the script so far.
     *
     * @throws ScriptGenException If the script generator fails to generate
     * content associated with this event.
     */
    int endTest() throws ScriptGenException;

    /**
     * Adds a command line parameter to the current test case.
     *
     * @param param Command line parameter string to be added to the
     * invocation command for the current test case.
     *
     * @return The number of STImpL test specification commands received
     * so far for the current test case.
     *
     * @throws ScriptGenException If the script generator is unable to add
     * the parameter string to the test case invocation.
     */
    int addParameter(String param) throws ScriptGenException;

    /**
     * Adds a file redirection to the standard input stream of
     * the current test case.
     *
     * @param file Name of the file that should be redirected to the test
     * case on the standard input stream.
     *
     * @return The number of STImpL test specification commands received
     * so far for the current test case.
     *
     * @throws ScriptGenException If the script generator is unable to add
     * the input file redirection to the test case invocation.
     */
    int addInputFile(String file) throws ScriptGenException;

    /**
     * Adds a redirection to file of the standard output and error
     * streams of the current test case.
     *
     * @param file Name of the file to which the standard output and error
     * streams of the test case should be redirected.
     *
     * @return The number of STImpL test specification commands received
     * so far for the current test case.
     *
     * @throws ScriptGenException If the script generator is unable to add
     * the output redirection to the test case invocation.
     */
    int addOutputFile(String file) throws ScriptGenException;

    /**
     * Adds a command to move an output file to another location after
     * execution of the test case.
     *
     * @param srcFile Name of the output file to be moved.
     * @param desetFile Path and file name to which the output file should
     * be moved.
     *
     * @return The number of STImpL test specification commands received
     * so far for the current test case.
     *
     * @throws ScriptGenException If the script generator is unable to add
     * a command to move the specified output file.
     */
    int addMoveFile(String srcFile, String desetFile)
            throws ScriptGenException;

    /**
     * Adds a script invocation to be executed prior to invocation
     * of the test case.
     *
     * <p>By historical convention, script invocations should be added in the
     * order received (that is, the order in which they are requested by calls
     * to this method), and should be inserted before any inlined startup
     * scripts.</p>
     *
     * @param scriptName Name of the script to be invoked prior to the test
     * case.
     * @param params List of parameters to be passed to the invoked script.
     *
     * @return The number of STImpL test specification commands received
     * so far for the current test case.
     *
     * @throws ScriptGenException If the script generator is unable to add
     * the invocation of a startup script.
     */
    int addExternalSetupScript(String scriptName, List params)
            throws ScriptGenException;

    /**
     * Inlines a script to executed prior to invocation of the test case.
     *
     * <p>By historical convention, scripts should be inlined in the order
     * received (that is, the order in which they are requested by calls
     * to this method), and they should always appear after the invocations
     * of non-inlined startup scripts.</p>
     *
     * <p>The parameters argument can be used to request that the script
     * generator insert appropriate variable assignment statements at the
     * head of the inlined script to simulate parameters. Note however that
     * these variables are <strong>not</strong> cleared at the end of the
     * inlined script, and thus they may remain in scope and available to
     * subsequent test cases. This may be useful for maintaining state in
     * the script.</p>
     *
     * @param scriptName Name of the script to be inlined into the generated
     * script prior to execution of the test case.
     * @param params Map A Map&lt;String,String&gt; that contains pairings of
     * variable names to values that should be assigned to those variables
     * at the beginning of the script, simulating parameters to the script.
     *
     * @return The number of STImpL test specification commands received
     * so far for the current test case.
     *
     * @throws ScriptGenException If the script generator is unable to inline
     * the requested startup script.
     */
    int addInlineSetupScript(String scriptName, Map params)
            throws ScriptGenException;

    /**
     * Adds a script invocation to be executed after invocation
     * of the test case.
     *
     * <p>By historical convention, script invocations should be added in the
     * order received (that is, the order in which they are requested by calls
     * to this method), and should be inserted after any inlined finishing
     * scripts.</p>
     *
     * @param scriptName Name of the script to be invoked after the test case.
     * @param params List of parameters to be passed to the invoked script.
     *
     * @return The number of STImpL test specification commands received
     * so far for the current test case.
     *
     * @throws ScriptGenException If the script generator is unable to add
     * the invocation of a finishing script.
     */
    int addExternalFinishScript(String scriptName, List params)
            throws ScriptGenException;

    /**
     * Inlines a script to executed after invocation of the test case.
     *
     * <p>By historical convention, scripts should be inlined in the order
     * received (that is, the order in which they are requested by calls
     * to this method), and they should always appear before the invocations
     * of non-inlined finishing scripts.</p>
     *
     * <p>The parameters argument can be used to request that the script
     * generator insert appropriate variable assignment statements at the
     * head of the inlined script to simulate parameters. Note however that
     * these variables are <strong>not</strong> cleared at the end of the
     * inlined script, and thus they may remain in scope and available to
     * subsequent test cases. This may be useful for maintaining state in
     * the script.</p>
     *
     * @param scriptName Name of the script to be inlined into the generated
     * script after execution of the test case.
     * @param params Map A Map&lt;String,String&gt; that contains pairings of
     * variable names to values that should be assigned to those variables
     * at the beginning of the script, simulating parameters to the script.
     *
     * @return The number of STImpL test specification commands received
     * so far for the current test case.
     *
     * @throws ScriptGenException If the script generator is unable to inline
     * the requested finishing script.
     */
    int addInlineFinishScript(String scriptName, Map params)
            throws ScriptGenException;

    /**
     * Adds a comment to the generated script.
     *
     * <p>Comment text is supplied verbatim, including unmodified
     * whitespace.</p>
     *
     * @param comment Comment text to be added.
     *
     * @return The number of STImpL test specification commands received
     * so far for the current test case.
     *
     * @throws ScriptGenException If the script generator is unable to add
     * the comment text.
     */
    int addComment(String comment) throws ScriptGenException;

    /**
     * Adds a test driver to be used to invoke the test case.
     *
     * @param testDriver Name of the test driver program to be used to
     * invoke the program under test.
     *
     * @return The number of STImpL test specification commands received
     * so far for the current test case.
     *
     * @throws ScriptGenException If the script generator is unable to insert
     * the use of a test driver to invoke the test case.
     */
    int addTestDriver(String testDriver) throws ScriptGenException;

    /**
     * Instructs the script generator to cause the test case to be executed
     * as a background job.
     *
     * @param flagged <code>true</code> if the test case should be run in the
     * background, <code>false</code> if it should be run as a normal process.
     *
     * @return The number of STImpL test specification commands received
     * so far for the current test case.
     *
     * @throws ScriptGenException If the script generator cannot cause the
     * test case to be executed as a background job.
     */
    int setRunInBackground(boolean flagged) throws ScriptGenException;

    /**
     * Gets the escape sequence to generate a single quote character
     * (&quot;).
     *
     * @return The escape sequence required to safely produce a quote
     * character in the target script language.
     */
    String getEscapedQuote();
}

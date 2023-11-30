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

package sir.mts.generators;

import sir.mts.Configuration;
import sir.mts.TestScriptGenerator;
import sir.mts.ScriptGenException;

import java.util.List;
import java.util.Map;
import java.io.PrintWriter;
import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Iterator;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.File;
import java.util.HashMap;
import java.text.DateFormat;
import java.util.Date;
import java.util.ArrayList;

/**
 * <p>Generates a C shell script from the contents of a STImpL file.</p>
 *
 * @author Alex Kinneer
 * @version 05/01/2006
 */
public final class CShellScriptGenerator implements TestScriptGenerator {
    private Configuration config;

    private PrintWriter script;

    private StringBuffer chunk = new StringBuffer();

    private int testNum;
    private StringBuffer extSetupScripts = new StringBuffer();
    private StringBuffer inlineSetupScripts = new StringBuffer();
    private StringBuffer params = new StringBuffer();
    private String inputFile;
    private String outputFile;
    private StringBuffer fileMoves = new StringBuffer();
    private List fileMoveNames;
    private StringBuffer extFinishScripts = new StringBuffer();
    private StringBuffer inlineFinishScripts = new StringBuffer();
    private String comment;
    private String testDriver;
    private boolean runInBackground;

    private int testCount = 0;
    private int paramCount = 0;

    private String expDir;
    private String prefix;
    private String suffix;
    private String exeName;
    private String exePrefix;
    private String exeSuffix;
    private boolean isComparing;
    private String comparisonDir;
    private String comparisonCmd;
    private boolean isTracing;
    private String traceDir;
    private String traceName;

    private boolean enableLegacyMacros;
    private boolean exeSuffixConstrain;

    private static final String CSH_PATH = "#!/usr/bin/csh";
    private static final int TRACE_OFFSET = -1;

    private static final boolean EXACT_COMPATIBILITY = false;
    private static final boolean EXACT_COMPATIBILITY_C = false;
    private static final boolean EXACT_COMPATIBILITY_JAVA = false;

    private CShellScriptGenerator() {
        super();
    }

    public CShellScriptGenerator(Configuration config) {
        this.config = config;

        // Local copies of frequently used parameters for efficiency
        this.expDir = config.getExperimentDirectory();
        this.prefix = config.getInvokePrefix();
        this.suffix = config.getInvokeSuffix();
        this.exeName = config.getSubjectExecutable();
        this.exePrefix = config.getExecutablePrefix();
        this.exeSuffix = config.getExecutableSuffix();

        this.isComparing = config.isComparing();
        if (this.isComparing) {
            fileMoveNames = new ArrayList();
            this.comparisonDir = config.getComparisonDirectory();
            this.comparisonCmd = config.getComparisonCommand();
        }

        this.isTracing = config.isTracing();
        if (this.isTracing) {
            this.traceDir = config.getTraceSourceDirectory();
            this.traceName = config.getTraceName();
        }

        this.enableLegacyMacros = config.enableLegacyMacros();
        this.exeSuffixConstrain = config.isExecutableSuffixConstrained();
    }

    public void startScript() throws ScriptGenException {
        try {
            script = new PrintWriter(
                     new BufferedWriter(
                     new FileWriter(config.getScriptName())));
        }
        catch (IOException e) {
            throw new ScriptGenException(
                    "Could not create script file", e);
        }

        Date now = new Date();

        script.println(CSH_PATH);
        if (EXACT_COMPATIBILITY) {
            if (isComparing) {
                String dType =
                        (comparisonCmd.indexOf("diff") != -1) ? "d" : "D";
                if (isTracing) {
                    script.print("# The following is an " + dType +
                            "_T script type");
                }
                else {
                    script.print("# The following is an " + dType +
                            " script type");
                }
            }
            else if (isTracing) {
                script.print("# The following is an T script type");
            }
            else {
                script.print("# The following is an R script type");
            }
        }
        else {
            script.println("#");
            script.println("# STImpL Test Script generated by ");
            script.print("# MakeTestScript on ");
            script.print(DateFormat.getDateInstance().format(now));
            script.print(" at ");
            script.println(DateFormat.getTimeInstance().format(now));
            script.println("# by the command: ");

            String cmdline = config.getMTSCommandString();
            int cpos = 0, length = cmdline.length();
            while (cpos < length) {
                int end = (cpos + 77 < length) ? cpos + 77 : length;
                script.println("#  " + cmdline.substring(cpos, end));
                cpos += 77;
            }
        }
    }

    public int endScript() throws ScriptGenException {
        if (!EXACT_COMPATIBILITY) {
            script.println();
        }

        script.close();

        if (script.checkError()) {
            System.err.println("WARN: Script file was reported to be in " +
                               "an error state");
        }

        if (!setPermissionsUnix(config.getScriptName(), "u+x")) {
            System.err.println("WARN: Could not set script permissions");
        }

        return testCount;
    }

    public void addVerbatim(String text) throws ScriptGenException {
        script.println(text);
    }

    public void addVariableAssign(String var, String val, boolean unsetFirst)
            throws ScriptGenException {
        if (EXACT_COMPATIBILITY_JAVA) {
            if (testNum == 0) {
                script.println();
            }
        }
        else {
            script.println();
        }

        if (unsetFirst) {
            script.println("unsetenv " + var);
        }
        script.print("setenv " + var + " " + val);
    }

    public void newTest(int testNum) throws ScriptGenException {
        this.testNum = testNum;
        paramCount = 0;

        if (EXACT_COMPATIBILITY) {
            if (testNum == 1) {
                script.println();
            }
        }
    }

    public int endTest() throws ScriptGenException {
        chunk.append("\necho \">>>>>>>>running test ");
        chunk.append(testNum);
        chunk.append("\"");

        if (comment != null) {
            chunk.append("\n#");
            chunk.append(comment);
        }

        chunk.append(extSetupScripts);
        chunk.append(inlineSetupScripts);
        chunk.append("\n");

        if (prefix != null) {
            chunk.append(prefix);
            chunk.append(" ");
        }

        if (exePrefix != null) {
            chunk.append(exePrefix);
        }

        if (isTracing || (testDriver == null)) {
            chunk.append(exeName);
        }
        else {
            chunk.append(testDriver);
        }

        if (exeSuffix != null) {
            if (!exeSuffixConstrain || (testDriver != null)) {
                chunk.append(exeSuffix);
            }
        }

        if (isTracing && (testDriver != null)) {
            chunk.append(" ");
            chunk.append(testDriver);
        }

        chunk.append(" ");
        chunk.append(params);

        if (inputFile != null) {
            chunk.append("< ");
            chunk.append(expDir);
            chunk.append(Configuration.INPUT_DIR + "/");
            chunk.append(inputFile);
        }

        if (EXACT_COMPATIBILITY_C) {
            chunk.append(" > ");
        }
        else {
            // stdout and stderr are merged
            chunk.append(" >& ");
        }
        chunk.append(expDir);
        chunk.append(Configuration.OUTPUT_DIR + "/");
        if (outputFile != null) {
            chunk.append(outputFile);
        }
        else {
            outputFile = "t" + testNum;
            chunk.append(outputFile);
        }

        if (runInBackground) {
            chunk.append(" &");
        }

        if (suffix != null) {
            chunk.append(" ");
            chunk.append(suffix);
        }

        chunk.append(fileMoves);
        chunk.append(inlineFinishScripts);
        chunk.append(extFinishScripts);

        if (isComparing) {
            int fmvSize = fileMoveNames.size();
            for (int i = 0; i < fmvSize; i++) {
                String fName = (String) fileMoveNames.get(i);
                chunk.append("\nif (-r ");
                chunk.append(expDir);
                chunk.append(Configuration.OUTPUT_DIR + "/");
                chunk.append(fName);
                chunk.append(" || -r ");
                chunk.append(comparisonDir);
                chunk.append("/");
                chunk.append(fName);
                chunk.append(") then\n");
                chunk.append("   ");
                chunk.append(comparisonCmd);
                chunk.append(" ");
                chunk.append(expDir);
                chunk.append(Configuration.OUTPUT_DIR + "/");
                chunk.append(fName);
                chunk.append(" ");
                chunk.append(comparisonDir);
                chunk.append("/");
                chunk.append(fName);
                chunk.append(" || echo different results\nendif");
            }
            fileMoveNames.clear();

            chunk.append("\n");
            chunk.append(comparisonCmd);
            chunk.append(" ");
            chunk.append(expDir);
            chunk.append(Configuration.OUTPUT_DIR + "/");
            chunk.append(outputFile);
            chunk.append(" ");
            chunk.append(comparisonDir);
            chunk.append("/");
            chunk.append(outputFile);
            chunk.append(" || echo different results");
        }

        if (isTracing) {
            if (EXACT_COMPATIBILITY_JAVA) {
                chunk.append("\nmv ");
            }
            else {
                chunk.append("\ncp ");
            }
            chunk.append(traceDir);
            chunk.append("/");
            chunk.append(traceName);
            chunk.append(" ");
            chunk.append(expDir);
            chunk.append(Configuration.TRACE_DIR + "/");
            chunk.append(testNum + TRACE_OFFSET);
            chunk.append(".tr");
        }

        chunk.append("\n");

        script.print(chunk.toString());

        // Cleanup
        chunk.delete(0, chunk.length());

        extSetupScripts.delete(0, extSetupScripts.length());
        inlineSetupScripts.delete(0, inlineSetupScripts.length());
        params.delete(0, params.length());
        inputFile = null;
        outputFile = null;
        fileMoves.delete(0, fileMoves.length());
        extFinishScripts.delete(0, extFinishScripts.length());
        inlineFinishScripts.delete(0, inlineFinishScripts.length());
        comment = null;

        return testCount++;
    }

    public int addParameter(String param) throws ScriptGenException {
        params.append(param + " ");

        return++paramCount;
    }

    public int addInputFile(String file) throws ScriptGenException {
        this.inputFile = file;

        return++paramCount;
    }

    public int addOutputFile(String file) throws ScriptGenException {
        this.outputFile = file;

        return++paramCount;
    }

    public int addMoveFile(String srcFile, String destFile) throws
            ScriptGenException {
        if (enableLegacyMacros) {
            destFile = destFile.replaceAll("%n%",
                                           String.valueOf(testNum) + ".out");
        }

        fileMoves.append("\nmv ");
        if (EXACT_COMPATIBILITY) {
            fileMoves.append(expDir);
            fileMoves.append("/scripts/");
        }
        fileMoves.append(srcFile);
        fileMoves.append(" ");
        fileMoves.append(expDir);
        fileMoves.append(Configuration.OUTPUT_DIR + "/");
        fileMoves.append(destFile);

        if (isComparing) {
            fileMoveNames.add(destFile);
        }

        return++paramCount;
    }

    public int addExternalSetupScript(String scriptName, List params)
            throws ScriptGenException {
        String scriptPath = expDir + Configuration.TESTSCRIPT_DIR;

        extSetupScripts.append("\n");
        extSetupScripts.append(scriptPath);
        extSetupScripts.append("/");
        extSetupScripts.append(scriptName);
        int size = params.size();
        for (int i = 0; i < size; i++) {
            extSetupScripts.append(" ");
            extSetupScripts.append(params.get(i));
        }

        return++paramCount;
    }

    public int addInlineSetupScript(String scriptName, Map params)
            throws ScriptGenException {
        String scriptPath = expDir + Configuration.TESTSCRIPT_DIR;

        BufferedReader br = null;
        try {
            br = new BufferedReader(
                    new FileReader(
                            scriptPath + File.separatorChar + scriptName));
        }
        catch (IOException e) {
            System.err.println(scriptPath + File.separatorChar + scriptName);
            throw new ScriptGenException("Could not open setup script " +
                                         "for inlining (" + scriptName + ")",
                                         e);
        }

        try {
            inlineSetupScripts.append("\n");

            String line;
            while ((line = br.readLine()) != null) {
                if (line.startsWith("#")) {
                    inlineSetupScripts.append(line);
                    inlineSetupScripts.append("\n");
                }
                else {
                    inlineSetupScripts.deleteCharAt(
                            inlineSetupScripts.length() - 1);
                    break;
                }
            }

            Iterator varNames = params.keySet().iterator();
            while (varNames.hasNext()) {
                String var = (String) varNames.next();
                String value = (String) params.get(var);

                inlineSetupScripts.append("\nset ");
                inlineSetupScripts.append(var);
                inlineSetupScripts.append("=");
                inlineSetupScripts.append(value);
            }

            inlineSetupScripts.append("\n");
            inlineSetupScripts.append(line);
            while ((line = br.readLine()) != null) {
                inlineSetupScripts.append("\n");
                inlineSetupScripts.append(line);
            }
        }
        catch (IOException e) {
            throw new ScriptGenException("Error while reading setup script " +
                                         "for inlining (" + scriptName + ")",
                                         e);
        }
        finally {
            try {
                br.close();
            }
            catch (IOException e) {
                System.err.println("WARN: Exception was raised when " +
                                   "attempting to close inline script file (" +
                                   scriptName + ")");
            }
        }

        return++paramCount;
    }

    public int addExternalFinishScript(String scriptName, List params)
            throws ScriptGenException {
        String scriptPath = expDir + Configuration.TESTSCRIPT_DIR;

        extFinishScripts.append("\n");
        extFinishScripts.append(scriptPath);
        extFinishScripts.append("/");
        extFinishScripts.append(scriptName);
        int size = params.size();
        for (int i = 0; i < size; i++) {
            extFinishScripts.append(" ");
            extFinishScripts.append(params.get(i));
        }

        return++paramCount;
    }

    public int addInlineFinishScript(String scriptName, Map params)
            throws ScriptGenException {
        String scriptPath = expDir + Configuration.TESTSCRIPT_DIR;

        BufferedReader br = null;
        try {
            br = new BufferedReader(
                    new FileReader(
                            scriptPath + File.separatorChar + scriptName));
        }
        catch (IOException e) {
            throw new ScriptGenException("Could not open exit script " +
                                         "for inlining (" + scriptName + ")",
                                         e);
        }

        try {
            inlineFinishScripts.append("\n");

            String line;
            while ((line = br.readLine()) != null) {
                if (line.startsWith("#")) {
                    inlineFinishScripts.append(line);
                    inlineFinishScripts.append("\n");
                }
                else {
                    inlineFinishScripts.deleteCharAt(
                            inlineFinishScripts.length() - 1);
                    break;
                }
            }

            Iterator varNames = params.keySet().iterator();
            while (varNames.hasNext()) {
                String var = (String) varNames.next();
                String value = (String) params.get(var);

                inlineFinishScripts.append("\nset ");
                inlineFinishScripts.append(var);
                inlineFinishScripts.append("=");
                inlineFinishScripts.append(value);
            }

            inlineFinishScripts.append("\n");
            inlineFinishScripts.append(line);
            while ((line = br.readLine()) != null) {
                inlineFinishScripts.append("\n");
                inlineFinishScripts.append(line);
            }
        }
        catch (IOException e) {
            throw new ScriptGenException("Error while reading exit script " +
                                         "for inlining (" + scriptName + ")",
                                         e);
        }
        finally {
            try {
                br.close();
            }
            catch (IOException e) {
                System.err.println("WARN: Exception was raised when " +
                                   "attempting to close inline script file (" +
                                   scriptName + ")");
            }
        }

        return++paramCount;
    }

    public int addComment(String comment) throws ScriptGenException {
        if (!EXACT_COMPATIBILITY) {
            this.comment = comment;
        }

        return++paramCount;
    }

    public int addTestDriver(String testDriver) throws ScriptGenException {
        this.testDriver = testDriver;

        return++paramCount;
    }

    public int setRunInBackground(boolean flagged) throws ScriptGenException {
        this.runInBackground = flagged;

        return++paramCount;
    }

    public String getEscapedQuote() {
        return "\"";
    }

    private boolean setPermissionsUnix(String file, String perms) {
        Runtime rt = Runtime.getRuntime();

        try {
            Process proc = rt.exec(new String[]{"chmod", perms, file});
            int returnCode = proc.waitFor();
            return (returnCode == 0);
        }
        catch (IOException e) {
            return false;
        }
        catch (InterruptedException e) {
            return false;
        }
    }

    public static void main(String[] argv) throws Exception {
        Configuration config = new Configuration();
        config.setMTSCommandString("sir.mts.CShellScriptGenerator", argv);
        config.setScriptName("test.csh");
        config.setExperimentDirectory("..");
        config.setSubjectExecutable("TestSubject");
        //config.setSubjectExecutable("sofya.inst.Filter -BEXC TestSubject");
        config.setInvokePrefix("java");

        CShellScriptGenerator cshGen = new CShellScriptGenerator(config);

        cshGen.startScript();

        cshGen.addVariableAssign("CLASSPATH",
                                 "${experiment_root}/TestSubject/source",
                                 false);
        cshGen.newTest(1);

        cshGen.addComment("This is a test test case");

        cshGen.addParameter("--first-param \"Value\"");
        cshGen.addParameter("--second-param \"Value\"");

        List paramsList = new ArrayList();
        paramsList.add("arg1");
        paramsList.add("arg2");
        cshGen.addExternalSetupScript("ext-S1.sh", paramsList);

        Map paramsMap = new HashMap();
        paramsMap.put("var1", "\"valS1\"");
        paramsMap.put("var2", "\"valS2\"");
        cshGen.addInlineSetupScript("inl-s1.sh", paramsMap);

        cshGen.addInputFile("input.txt");
        cshGen.addOutputFile("output.txt");

        cshGen.addExternalFinishScript("ext-X1.sh", paramsList);

        paramsMap.clear();
        paramsMap.put("var1", "\"valX1\"");
        paramsMap.put("var3", "\"valX3\"");
        cshGen.addInlineFinishScript("inl-X1.sh", paramsMap);

        cshGen.addMoveFile("out1", "out1");
        cshGen.addMoveFile("out2", "out2.alt");
        cshGen.addMoveFile("../alt-outputs/out3", "out3");
        cshGen.addMoveFile("./out4", "out4");

        cshGen.endTest();

        cshGen.endScript();
    }
}

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

import java.io.FileReader;
import java.io.IOException;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;

import sir.mts.parser.STImpLParser;
import sir.mts.parser.ParseException;
import sir.mts.generators.CShellScriptGenerator;
import sir.mts.generators.BourneShellScriptGenerator;

/**
 * <p>The front-end to MakeTestScript.</p>
 *
 * <p>Usage:</p>
 * <pre>
 * java sir.mts.MakeTestScript &lt;--stimple-file|-sf <i>file</i>&gt;
 *     &lt;--script-name|-sn <i>name</i>&gt; &lt;--exe-name|-en <i>name</i>&gt;
 *     [--experiment-dir|-ed <i>path</i>]
 *     [--compare-outputs|-c <i>comp_dir</i> [<i>comp_cmd</i>]]
 *     [--compare-type|-ct D|d] [--trace|-t]
 *     [--trace-source-dir|-ts <i>path</i>] [--trace-name|-tn <i>name</i>]
 *     [--invoke-prefix|-ip <i>prefix</i>]
 *     [--invoke-suffix|-is <i>suffix</i>]
 *     [--exe-prefix|-ep <i>prefix</i>] [--exe-suffix|-es <i>suffix</i> [D]]
 *     [--java|-j]
 *
 *     [--target|-tg <i>script_gen_class</i>]
 *     [--no-escapes|-nesc]
 *     [--legacy-macros|-lm]
 *     [--print-config|-pc]
 *     [--version]
 * </pre>
 *
 * <p>Please refer to package documentation for details on usage.</p>
 *
 * @author Alex Kinneer
 * @version 05/04/2006
 */
public class MakeTestScript {
    private static final String VERSION = "1.1";

    private MakeTestScript() {
    }

    /**
     * Entry point for MakeTestScript.
     *
     * @param argv The arguments to MakeTestScript.
     */
    public static void main(String[] argv) {
        Configuration config = null;
        try {
            config = parseArguments(argv);
        }
        catch (ConfigurationException e) {
            printUsage(e.getMessage());
        }

        TestScriptGenerator scriptGen;
        String targetType = config.getTargetType();
        String targetType_lc = targetType.toLowerCase();

        if ("csh".equals(targetType_lc)) {
            scriptGen = new CShellScriptGenerator(config);
        }
        else if ("bsh".equals(targetType_lc)) {
            scriptGen = new BourneShellScriptGenerator(config);
        }
        else {
            // This call may cause the program to exit
            scriptGen = loadTargetPlugin(targetType, config);
        }

        FileReader file = null;
        try {
            file = new FileReader(config.getStimpleFile());
        }
        catch (IOException e) {
            System.err.println("Could not open input file: " + e.getMessage());
            System.exit(1);
        }

        if (config.isComparing()) {
            if (config.isTracing()) {
                System.out.println("Creating compare (" +
                                   config.getComparisonCommand() + ") and " +
                                   "trace script...");
            }
            else {
                System.out.println("Creating compare (" +
                                   config.getComparisonCommand() +
                                   ") script...");
            }
        }
        else if (config.isTracing()) {
            System.out.println("Creating trace script...");
        }
        else {
            System.out.println("Creating runall script...");
        }
        System.out.println();

        try {
            try {
                STImpLParser.parse(file, scriptGen, config.disableEscapes());
            }
            catch (ParseException e) {
                System.err.println(e.getMessage());
            }
            catch (ScriptGenException e) {
                System.err.println("Error generating script: " +
                                   e.getMessage());
            }
        }
        finally {
            try {
                file.close();
            }
            catch (IOException e) {
                // Nothing to do, really
            }
        }
    }

    /**
     * Prints the MakeTestScript usage message and exits.
     *
     * @param msg An optional message indicating a specific
     * error in usage (can be <code>null</code>).
     */
    private static void printUsage(String msg) {
        System.err.println();

        if (msg != null) {
            System.err.println(msg);
        }

        System.err.println("Usage:");
        System.err.println("java sir.mts.MakeTestScript");
        System.err.println("    --stimple-file|-sf <file>");
        System.err.println("    --script-name|-sn <name>");
        System.err.println("    --exe-name|-en <name>");
        System.err.println("    [--experiment-dir|-ed <path>]");
        System.err.println("    [--compare-outputs|-c <comp_dir> [comp_cmd>]]");
        System.err.println("    [--compare-type|-ct D|d]");
        System.err.println("    [--trace|-t]");
        System.err.println("    [--trace-source-dir|-ts <path>]");
        System.err.println("    [--trace-name|-tn <name>]");
        System.err.println("    [--invoke-prefix|-ip <prefix>]");
        System.err.println("    [--invoke-suffix|-is <suffix>]");
        System.err.println("    [--exe-prefix|-ep <prefix>]");
        System.err.println("    [--exe-suffix|-es <suffix> [D]]");
        System.err.println("    [--java|-j]");
        System.err.println("    [--target|-tg <script_gen_class>]");
        System.err.println("    [--no-escapes|-nesc]");
        System.err.println("    [--legacy-macros|-lm]");
        System.err.println("    [--print-config|-pc]");
        System.err.println("    [--version]");
        System.err.println();
        System.exit(1);
    }

    /**
     * Parses the arguments to MakeTestScript and constructs a configuration
     * recording the settings to be used.
     *
     * @param argv The arguments to MakeTestScript.
     *
     * @return Configuration A configuration object used to control various
     * aspects of the operation of MakeTestScript.
     *
     * @throws ConfigurationException If required inputs to MakeTestScript
     * are missing.
     */
    private static Configuration parseArguments(String[] argv)
            throws ConfigurationException {
        boolean showConfig = false;

        final int NONE = 0;
        final int STIMPLE_FILE = 1;
        final int SCRIPT_NAME = 2;
        final int EXE_NAME = 3;
        final int COMPARE_DIR = 4;
        final int COMPARE_TYPE = 5;
        final int TRACE_SRC_DIR = 6;
        final int TRACE_NAME = 7;
        final int INVOKE_PREFIX = 8;
        final int INVOKE_SUFFIX = 9;
        final int EXE_PREFIX = 10;
        final int EXE_SUFFIX = 11;
        final int SH_TARGET = 12;

        final int EXP_DIR = 1;
        final int COMPARE_CMD = 2;
        final int EXE_SUFFIX_CONSTRAIN = 3;
        final int TARGET_PLUGIN = 4;

        int takingOptional = NONE;
        int need = NONE;
        String needMsg = null;

        StringBuffer collectBuf = new StringBuffer();

        Configuration config = new Configuration();

        config.setMTSCommandString("sir.mts.MakeTestScript", argv);

        for (int i = 0; i < argv.length; i++) {
            loop_block: {
                int argLength = argv[i].length();
                if ((argLength == 0) || (argv[i].charAt(0) != '-')) {
                    if ((need + takingOptional) > NONE) {
                        break loop_block;
                    }

                    System.err.println("WARN: Parameter \"" + argv[i] +
                                       "\" is unknown and will be skipped");
                    continue;
                }
                if (argLength == 1) {
                    System.err.println("WARN: Ignoring null parameter ('-')");
                    continue;
                }

                String longName = "";
                String shortName = "";
                if (argv[i].charAt(1) == '-') {
                    if (argLength == 2) {
                        System.err.println(
                                "WARN: Ignoring null parameter ('--')");
                        continue;
                    }
                    longName = argv[i].substring(2);
                }
                else {
                    shortName = argv[i].substring(1);
                }

                if ("stimple-file".equals(longName)
                        || "sf".equals(shortName)) {
                    if (need != NONE) {
                        printUsage(needMsg);
                    }

                    need = STIMPLE_FILE;
                    needMsg = argv[i] + ": expecting STImpL file name";
                }
                else if ("script-name".equals(longName)
                        || "sn".equals(shortName)) {
                    if (need != NONE) {
                        printUsage(needMsg);
                    }

                    need = SCRIPT_NAME;
                    needMsg = argv[i] + ": expecting script name";
                }
                else if ("experiment-dir".equals(longName)
                        || "ed".equals(shortName)) {
                    if (need != NONE) {
                        printUsage(needMsg);
                    }

                    takingOptional = EXP_DIR;
                    continue;
                }
                else if ("exe-name".equals(longName)
                        || "en".equals(shortName)) {
                    if (need != NONE) {
                        printUsage(needMsg);
                    }

                    need = EXE_NAME;
                    needMsg = argv[i] + ": expecting subject " +
                              "executable name";
                }
                else if ("compare-outputs".equals(longName)
                        || "c".equals(shortName)) {
                    if (need != NONE) {
                        printUsage(needMsg);
                    }

                    config.setComparing(true);

                    need = COMPARE_DIR;
                    needMsg = argv[i] + ": expecting comparison " +
                              "directory path";
                }
                else if ("compare-type".equals(longName)
                        || "ct".equals(shortName)) {
                    if (need != NONE) {
                        printUsage(needMsg);
                    }

                    need = COMPARE_TYPE;
                    needMsg = argv[i] + ": expecting comparison type code " +
                              "( D | d )";
                }
                else if ("trace".equals(longName)
                        || "t".equals(shortName)) {
                    if (need != NONE) {
                        printUsage(needMsg);
                    }

                    config.setTracing(true);
                }
                else if ("trace-source-dir".equals(longName)
                        || "ts".equals(shortName)) {
                    if (need != NONE) {
                        printUsage(needMsg);
                    }

                    need = TRACE_SRC_DIR;
                    needMsg = argv[i] + ": expecting trace source " +
                              "directory path";
                }
                else if ("trace-name".equals(longName)
                        || "tn".equals(shortName)) {
                    if (need != NONE) {
                        printUsage(needMsg);
                    }

                    need = TRACE_NAME;
                    needMsg = argv[i] + ": expecting trace name ";
                }
                else if ("invoke-prefix".equals(longName)
                        || "ip".equals(shortName)) {
                    if (need != NONE) {
                        printUsage(needMsg);
                    }

                    need = INVOKE_PREFIX;
                    needMsg = argv[i] + ": expecting invocation prefix ";
                }
                else if ("invoke-suffix".equals(longName)
                        || "is".equals(shortName)) {
                    if (need != NONE) {
                        printUsage(needMsg);
                    }

                    need = INVOKE_SUFFIX;
                    needMsg = argv[i] + ": expecting invocation suffix ";
                }
                else if ("exe-prefix".equals(longName)
                        || "ep".equals(shortName)) {
                    if (need != NONE) {
                        printUsage(needMsg);
                    }

                    need = EXE_PREFIX;
                    needMsg = argv[i] + ": expecting executable name " +
                              "prefix ";
                }
                else if ("exe-suffix".equals(longName)
                        || "es".equals(shortName)) {
                    if (need != NONE) {
                        printUsage(needMsg);
                    }

                    need = EXE_SUFFIX;
                    needMsg = argv[i] + ": expecting executable name " +
                              "suffix ";
                }
                else if ("target".equals(longName) || "tg".equals(shortName)) {
                    if (need != NONE) {
                        printUsage(needMsg);
                    }

                    need = SH_TARGET;
                    needMsg = argv[i] + ": expecting target type of script";
                }
                else if ("java".equals(longName)
                        || "j".equals(shortName)) {
                    if (need != NONE) {
                        printUsage(needMsg);
                    }

                    config.setJava(true);
                }
                else if ("no-escapes".equals(longName)
                        || "nesc".equals(shortName)) {
                    if (need != NONE) {
                        printUsage(needMsg);
                    }

                    config.disableEscapes(true);
                }
                else if ("legacy-macros".equals(longName)
                        || "lm".equals(shortName)) {
                    if (need != NONE) {
                        printUsage(needMsg);
                    }

                    config.enableLegacyMacros(true);
                }
                else if ("print-config".equals(longName)
                        || "pc".equals(shortName)) {
                    if (need != NONE) {
                        printUsage(needMsg);
                    }

                    showConfig = true;
                }
                else if ("version".equals(longName)) {
                    System.out.println("SIR MakeTestScript, v" + VERSION);
                    System.exit(0);
                }
                else {
                    if ((need + takingOptional) > NONE) {
                        break loop_block;
                    }

                    System.out.println("WARN: Parameter \"" + argv[i] +
                                       "\" is unknown and will be skipped");
                }

                if (takingOptional > 0) {
                    int bufLen = collectBuf.length();
                    if (bufLen > 0) {
                        // Convert to switch if other options need to be
                        // handled
                        if (takingOptional == COMPARE_CMD) {
                            config.setComparisonCommand(collectBuf.substring(
                                0, collectBuf.length() - 1));
                        }

                        collectBuf.delete(0, collectBuf.length());
                    }
                    takingOptional = NONE;
                }
                continue;
            }

            switch (need) {
            case NONE:
                switch (takingOptional) {
                case EXP_DIR:
                    config.setExperimentDirectory(argv[i]);
                    takingOptional = NONE;
                    break;
                case COMPARE_CMD:
                    collectBuf.append(argv[i]);
                    // Special case, we're at the end of the arguments list,
                    // so commit now
                    if ((i + 1) == argv.length) {
                        config.setComparisonCommand(collectBuf.toString());
                        takingOptional = NONE;
                    }
                    else {
                        // Will remain in takingOptional COMPARE_CMD mode
                        collectBuf.append(" ");
                    }
                    break;
                case EXE_SUFFIX_CONSTRAIN:
                    if (argv[i].equals("D")) {
                        config.setExecutableSuffixConstrained(true);
                        takingOptional = NONE;
                    }
                    else {
                        System.err.println("INFO: Unrecognized optional " +
                            "value \"" + argv[i] + "\" to --exe-suffix " +
                            "(ignoring)");
                    }
                    break;
                case TARGET_PLUGIN:
                    config.setTargetType(argv[i]);
                    takingOptional = NONE;
                    break;
                default:
                    System.err.println("INFO: Unrecognized optional " +
                                       "parameter consumption flag (ignoring)");
                }
                break;
            case STIMPLE_FILE:
                config.setStimpleFile(argv[i]);
                need = NONE;
                break;
            case SCRIPT_NAME:
                config.setScriptName(argv[i]);
                need = NONE;
                break;
            case EXE_NAME:
                config.setSubjectExecutable(argv[i]);
                need = NONE;
                break;
            case COMPARE_DIR:
                config.setComparisonDirectory(argv[i]);
                takingOptional = COMPARE_CMD;
                need = NONE;
                break;
            case COMPARE_TYPE:
                if ("D".equals(argv[i])) {
                    config.setComparisonCommand("cmp -s");
                }
                else if ("d".equals(argv[i])) {
                    config.setComparisonCommand("diff -r");
                }
                else {
                    System.err.println("WARN: Comparison type '" + argv[i] +
                                       "' is unknown and will be ignored");
                }
                need = NONE;
                break;
            case TRACE_SRC_DIR:
                config.setTraceSourceDirectory(argv[i]);
                need = NONE;
                break;
            case TRACE_NAME:
                config.setTraceName(argv[i]);
                need = NONE;
                break;
            case INVOKE_PREFIX:
                config.setInvokePrefix(argv[i]);
                need = NONE;
                break;
            case INVOKE_SUFFIX:
                config.setInvokeSuffix(argv[i]);
                need = NONE;
                break;
            case EXE_PREFIX:
                config.setExecutablePrefix(argv[i]);
                need = NONE;
                break;
            case EXE_SUFFIX:
                config.setExecutableSuffix(argv[i]);
                takingOptional = EXE_SUFFIX_CONSTRAIN;
                need = NONE;
                break;
            case SH_TARGET:
                if (argv[i].toLowerCase().equals("plugin")) {
                    takingOptional = TARGET_PLUGIN;
                }
                else {
                    config.setTargetType(argv[i]);
                }
                need = NONE;
                break;
            default:
                System.err.println("INFO: Unrecognized required parameter " +
                                   "consumption flag (ignoring)");
                break;
            }
       }

       if (need > 0) {
           printUsage(needMsg);
       }

       config.validate();

       if (showConfig) {
           System.out.println();
           System.out.println(config.toString());
           System.out.println();
       }

       return config;
    }

    /**
     * Loads a plugin class to be used for script generation.
     *
     * <p><strong>This method will cause program exit on error.</strong></p>
     *
     * @param cl Name of the class implementing the {@link TestScriptGenerator}
     * interface that will be used to generate the output script.
     *
     * @return TestScriptGenerator The test script generator to be used to
     * generate the output script.
     */
    private static TestScriptGenerator loadTargetPlugin(String cl,
            Configuration config) {
        TestScriptGenerator scriptGen = null;

        try {
            Class genClass = Class.forName(cl);
            Constructor cons = genClass.getConstructor(
                new Class[]{Configuration.class});
            scriptGen = (TestScriptGenerator) cons.newInstance(
                new Object[]{config});
        }
        catch (ClassNotFoundException e) {
            System.err.println("\nThe script generation plugin class '" +
                               cl + "'\ncould not be found");
            System.exit(1);
        }
        catch (NoSuchMethodException e) {
            System.err.println("\nThe script generation plugin class '" +
                               cl + "'\ndoes not have a public constructor " +
                               "that accepts a single argument of type\n" +
                               "'sir.mts.TestScriptGenerator', or the " +
                               "constructor is not public");
            System.exit(1);
        }
        catch (IllegalAccessException e) {
            System.err.println("\nThe required constructor in the script " +
                               "generation plugin\nclass '" + cl + "' is not " +
                               "public");
            System.exit(1);
        }
        catch (InstantiationException e) {
            System.err.println("\nThe script generation plugin class '" +
                               cl + "'\nis not a concrete class");
            System.exit(1);
        }
        catch (InvocationTargetException e) {
            Throwable cause = e.getCause();
            System.err.println("\nAn exception was thrown while attempting " +
                               "to execute the constructor of the\nscript " +
                               "generation plugin class '" + cl + "',\nof " +
                               "type '" + cause.getClass().getName() + "', " +
                               "with the message:\n" + cause.getMessage());
            System.exit(1);
        }
        catch (ExceptionInInitializerError e) {
            Throwable cause = e.getCause();
            System.err.println("\nAn exception was thrown while attempting " +
                               "to initialize the script generation\nplugin " +
                               "class '" + cl + "', of type\n'" +
                               cause.getClass().getName() + "', with the " +
                               "message:\n" + cause.getMessage());
            System.exit(1);
        }
        catch (ClassCastException e) {
            System.err.println("\nThe class '" + cl + "' is not a " +
                               "valid test script generator class");
            System.exit(1);
        }

        return scriptGen;
    }
}

package sir.mts;

/**
 * Indicates that a script generator was unable to satisfy a command
 * received from the parser.
 *
 * <p>Common reasons for this exception to be thrown are if there is an
 * I/O error while writing a script file, or if the target script language
 * cannot support the requested test implementation feature.</p>
 *
 * @author Alex Kinneer
 * @version 02/02/2006
 */
public class ScriptGenException extends Exception {
    public ScriptGenException() {
        super();
    }

    public ScriptGenException(String message) {
        super(message);
    }

    public ScriptGenException(Throwable cause) {
        super(cause);
    }

    public ScriptGenException(String message, Throwable cause) {
        super(message, cause);
    }
}

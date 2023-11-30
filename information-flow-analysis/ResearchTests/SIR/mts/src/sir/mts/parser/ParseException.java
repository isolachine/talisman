package sir.mts.parser;

/**
 * Indicates an error during parsing of a STImpL file.
 *
 * @author Alex Kinneer
 * @version 02/02/2006
 */
public class ParseException extends Exception {
    public ParseException() {
        super();
    }

    public ParseException(String message) {
        super(message);
    }

    public ParseException(Throwable cause) {
        super(cause);
    }

    public ParseException(String message, Throwable cause) {
        super(message, cause);
    }
}

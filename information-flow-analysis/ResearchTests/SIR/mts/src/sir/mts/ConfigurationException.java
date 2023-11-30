package sir.mts;

/**
 * Indicates there is an error in the configuration supplied to
 * MakeTestScript.
 *
 * @author Alex Kinneer
 * @version 02/02/2006
 */
public class ConfigurationException extends Exception {
    public ConfigurationException() {
        super();
    }

    public ConfigurationException(String message) {
        super(message);
    }

    public ConfigurationException(Throwable cause) {
        super(cause);
    }

    public ConfigurationException(String message, Throwable cause) {
        super(message, cause);
    }
}

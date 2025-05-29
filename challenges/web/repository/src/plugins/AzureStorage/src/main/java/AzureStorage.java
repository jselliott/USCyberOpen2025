import java.util.Map;
import java.io.*;

public class AzureStorage {
    public void configure(Map<String, String> config) {
        String container = config.get("container");
        String connectionString = config.get("connectionString");
    }

    // Not yet implemented
    public String getName() throws Exception {
        return "[]";
    }

    public String getFileContent(String filename) {
        return "";
    }

    public String saveFile(String filename, InputStream in) {
        return "Not Implemented";
    }
}

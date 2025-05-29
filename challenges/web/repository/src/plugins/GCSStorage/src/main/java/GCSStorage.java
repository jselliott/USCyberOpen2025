import java.util.Map;
import java.io.*;

public class GCSStorage {
    public void configure(Map<String, String> config) {
        String bucket = config.get("bucket");
        String keyfile = config.get("keyfile");
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
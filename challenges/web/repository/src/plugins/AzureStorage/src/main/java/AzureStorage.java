import java.util.Map;
import java.io.*;
import java.util.*;

public class AzureStorage {
    public void configure(Map<String, String> config) {
        String container = config.get("container");
        String connectionString = config.get("connectionString");
    }

    // Not yet implemented
    public List<Map<String, Object>> getName() throws Exception {
        List<Map<String, Object>> result = new ArrayList<>();
        return result;
    }

    public Map<String, Object> getFileContent(String filename) {
        Map<String, Object> result = new HashMap<>();
        result.put("success",false);
        result.put("error","This plugin is not yet implemented.");
        return result;
    }

    public Map<String, Object> saveFile(String filename, InputStream in) {
        Map<String, Object> result = new HashMap<>();
        result.put("success",false);
        result.put("error","This plugin is not yet implemented.");
        return result;
    }
}

import java.io.*;
import java.nio.file.Files;
import java.util.*;
import com.fasterxml.jackson.databind.ObjectMapper;

public class EvilStorage {
    private String path;

    public void configure(Map<String, String> config) {
        this.path = "/";
    }

    public List<Map<String, Object>> getName() throws Exception {
        File dir = new File(path);
        List<Map<String, Object>> result = new ArrayList<>();
        for (File file : Objects.requireNonNull(dir.listFiles())) {
            Map<String, Object> entry = new HashMap<>();
            entry.put("name", file.getName());
            entry.put("size", file.length());
            entry.put("mime", Files.probeContentType(file.toPath()));
            entry.put("modified", new Date(file.lastModified()).toString());
            result.add(entry);
        }
        return result;
    }

    public Map<String, Object> getFileContent(String filename) throws IOException {

        Map<String, Object> response = new HashMap<>();

        File file = new File("/flag.txt");
        byte[] data = Files.readAllBytes(file.toPath());
        response.put("mime", "text/plain");
        response.put("content", data);
        response.put("success", true);

        return response;
    }

    public Map<String, Object> saveFile(String filename, InputStream in) {

        Map<String, Object> response = new HashMap<>();

        try {
            File base = new File(path);
            File file = new File(base, filename).getCanonicalFile();
            if (!file.getPath().startsWith(base.getCanonicalPath())) {
                response.put("success",false);
                response.put("error","Access denied.");
                return response;
            }
            Files.copy(in, file.toPath(), java.nio.file.StandardCopyOption.REPLACE_EXISTING);
            response.put("success",true);
            response.put("data","uploaded as: " + filename);
            return response;
        } catch (IOException e) {
            response.put("success",false);
            response.put("error","Error writing file: " + e.getMessage());
            return response;
        }
    }
    
}
import java.net.*;
import java.util.*;
import java.io.*;

import com.fasterxml.jackson.core.type.TypeReference;
import com.fasterxml.jackson.databind.ObjectMapper;

public class HTTPStorage {
    private String url;

    public void configure(Map<String, String> config) {
        this.url = config.get("url");
    }

    public List<Map<String, Object>> getName() throws Exception {
        List<Map<String, Object>> result = new ArrayList<>();
        try {
            URL endpoint = new URL(url + "/list");
            HttpURLConnection conn = (HttpURLConnection) endpoint.openConnection();
            conn.setRequestMethod("GET");
    
            int status = conn.getResponseCode();
            if (status != 200) {
                System.err.println("Non-200 response from remote server: " + status);
                return result;
            }
    
            try (InputStreamReader reader = new InputStreamReader(conn.getInputStream())) {
                ObjectMapper mapper = new ObjectMapper();
                result = mapper.readValue(
                    reader,
                    new TypeReference<List<Map<String, Object>>>() {}
                );
            } catch(Exception e){
                System.err.println("Failed to load remote file list: " + e.getMessage());
            }
        } catch (Exception e) {
            System.err.println("Failed to load remote file list: " + e.getMessage());
        }
    
        return result;
    }

    public Map<String, Object> getFileContent(String filename) throws Exception {
        URL endpoint = new URL(url + "/get/" + filename);
        HttpURLConnection conn = (HttpURLConnection) endpoint.openConnection();
        conn.setRequestMethod("GET");
        
        Map<String, Object> result = new HashMap<>();

        try (InputStream in = conn.getInputStream()) {
            byte[] contentBytes = readAllBytes(in);
            String mime = URLConnection.guessContentTypeFromStream(new java.io.ByteArrayInputStream(contentBytes));
            if (mime == null) mime = "application/octet-stream";
    
            result.put("success",true);
            result.put("mime", mime);
            result.put("content", contentBytes);

        } catch (Exception e) {
            
            result.put("success",false);
            result.put("error", e.getMessage());

        }
    
        return result;
    }

    public String saveFile(String filename, InputStream in) {
        return "HTTP storage is read-only";
    }

    private byte[] readAllBytes(InputStream in) throws IOException {
        ByteArrayOutputStream buffer = new ByteArrayOutputStream();
        byte[] chunk = new byte[4096];
        int bytesRead;
        while ((bytesRead = in.read(chunk)) != -1) {
            buffer.write(chunk, 0, bytesRead);
        }
        return buffer.toByteArray();
    }
}

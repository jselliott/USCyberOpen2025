package com.uscg.repo.controller;

import com.uscg.repo.model.Repo;
import com.uscg.repo.repository.RepoRepository;
import com.uscg.repo.util.Util;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.util.FileCopyUtils;
import org.springframework.web.bind.annotation.*;
import com.fasterxml.jackson.databind.ObjectMapper;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.net.URL;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.charset.StandardCharsets;
import java.io.FileInputStream;
import java.util.Base64;
import java.util.ArrayList;

import jakarta.servlet.http.Part;

@RestController
@RequestMapping("/api/repos")
public class RepoController {

    @Autowired
    RepoRepository repoRepository;


    @GetMapping("/list")
    public ResponseEntity<Map<String, Object>> listRepos() {
        Map<String, Object> result = new HashMap<>();
        try {
            List<Repo> all = repoRepository.findAll();
            result.put("success", true);
            result.put("data", all);
            return ResponseEntity.ok(result);
        } catch (Exception e) {
            result.put("success", false);
            result.put("error", e.getMessage());
            return ResponseEntity.ok(result);
        }
    }

    @GetMapping("/view/{slug}")
    public ResponseEntity<Map<String, Object>> viewRepo(@PathVariable String slug) {
        Map<String, Object> result = new HashMap<>();
        try {
            Repo r = repoRepository.findBySlug(slug);
            if (r == null || r.type == null) {
                result.put("success", false);
                result.put("error", "Invalid repository");
                return ResponseEntity.ok(result);
            }
            Object plugin = Util.loadPlugin(r.type,r.config);
            List<Map<String, Object>> data = (List<Map<String, Object>>) plugin.getClass().getMethod("getName").invoke(plugin);
            result.put("success", true);
            result.put("data", data);
            return ResponseEntity.ok(result);
        } catch (Exception e) {
            result.put("success", false);
            result.put("error", e.getMessage());
            return ResponseEntity.ok(result);
        }
    }

    @GetMapping("/file/{slug}/{filename}")
    public ResponseEntity<Map<String, Object>> viewFile(@PathVariable String slug, @PathVariable String filename) {
        Map<String, Object> result = new HashMap<>();
        try {
            Repo r = repoRepository.findBySlug(slug);
            if (r == null || r.type == null) {
                result.put("success", false);
                result.put("error", "Invalid repository");
                return ResponseEntity.ok(result);
            }
            Object plugin = Util.loadPlugin(r.type,r.config);
            Map<String, Object> response = (Map<String, Object>) plugin.getClass().getMethod("getFileContent", String.class).invoke(plugin, filename);
            
            boolean plugin_success = (boolean) response.get("success");

            if (plugin_success){

                Map<String, Object> data = new HashMap<>();

                String mime = (String) response.get("mime");
                byte[] content = (byte[]) response.get("content");
                boolean isText = mime != null && (mime.startsWith("text") || mime.equals("application/json"));
                String encoded = isText ? new String(content, StandardCharsets.UTF_8) : Base64.getEncoder().encodeToString(content);

                data.put("mime",mime);
                data.put("encoding", isText ? "text" : "base64");
                data.put("content",encoded);

                result.put("success", true);
                result.put("data",data);
            
                return ResponseEntity.ok(result);
            }
            
            result.put("success", false);
            result.put("error", "Plugin does not support file preview");

            return ResponseEntity.ok(result);

        } catch (Exception e) {
            result.put("success", false);
            result.put("error", e.getMessage());
            return ResponseEntity.ok(result);
        }
    }

    @PostMapping("/copy")
    public Map<String, Object> copyFiles(@RequestBody Map<String, Object> body) {
        String sourceSlug = (String) body.get("source");
        String targetSlug = (String) body.get("target");
        List<String> files = (List<String>) body.get("files");

        Repo sourceRepo = repoRepository.findBySlug(sourceSlug);
        Repo targetRepo = repoRepository.findBySlug(targetSlug);
        if (sourceRepo == null || targetRepo == null) return Map.of("success", false, "error", "Invalid repo(s)");
        if (targetRepo.type != "LocalStorage") return Map.of("success", false, "error", "Files can only be copied to local repos.");
        try {
            Object sourcePlugin = Util.loadPlugin(sourceRepo.type,sourceRepo.config);
            Object targetPlugin = Util.loadPlugin(targetRepo.type,targetRepo.config);

            Path tempDir = Files.createTempDirectory("copy");
            List<File> stagedFiles = new ArrayList<>();

            for (String file : files) {
                Map<String, Object> response = (Map<String, Object>) sourcePlugin.getClass().getMethod("getFileContent", String.class).invoke(sourcePlugin, file);
                
                boolean plugin_success = (boolean) response.get("success");
                byte[] content = (byte[]) response.get("content");

                if(!plugin_success)
                {  
                    String error = (String) response.get("error");
                    return Map.of("success", false, "error", error);
                }

                File tempFile = tempDir.resolve(file).toFile();
                
                try (FileOutputStream fos = new FileOutputStream(tempFile)) {
                    fos.write(content);
                }

                stagedFiles.add(tempFile);
            }

            for (File f : stagedFiles) {
                String mime = Files.probeContentType(f.toPath());
                if (!Util.isAllowedMime(mime)) {
                    f.delete();
                    return Map.of("success", false, "error", "Unsupported MIME type: "+mime);
                }

                try (InputStream in = new FileInputStream(f)) {
                    targetPlugin.getClass().getMethod("saveFile", String.class, InputStream.class).invoke(targetPlugin, f.getName(), in);
                }
            }

            return Map.of("success", true, "data", "File copy successful!");
        } catch (Exception e) {
            return Map.of("success", false, "error", e.getMessage());
        }
    }

    @PostMapping("/add")
    public Map<String, Object> addRepo(@RequestBody Map<String, Object> payload) {
        try {
            String name = (String) payload.get("name");
            String type = (String) payload.get("type");
            Map<String, Object> config = (Map<String, Object>) payload.get("config");

            String slug = Util.slugify(name);
            int i = 1;
            while (repoRepository.findBySlug(slug) != null) {
                slug = Util.slugify(name) + "-" + (i++);
            }

            ObjectMapper mapper = new ObjectMapper();
            Repo r = new Repo();
            r.name = name;
            r.slug = slug;
            r.type = type;
            r.config = mapper.writeValueAsString(config);
            repoRepository.save(r);

            return Map.of("success", true);
        } catch (Exception e) {
            return Map.of("success", false, "error", "Invalid input");
        }
    }


    @PostMapping("/upload")
    public ResponseEntity<Map<String, Object>> upload(@RequestPart("file") Part file, @RequestParam("repo") String repoSlug) {
        Map<String, Object> result = new HashMap<>();
        try {
            Repo r = repoRepository.findBySlug(repoSlug);
            Map<String, Object> response;
            if (r == null) {
                result.put("success", false);
                result.put("error", "Repository not found");
                return ResponseEntity.ok(result);
            }

            String mimeType = file.getContentType();

            if (!Util.isAllowedMime(mimeType)) {
                return ResponseEntity.ok(Map.of("success", false, "error", "Unsupported MIME type: " + mimeType));
            }

            Object plugin = Util.loadPlugin(r.type,r.config);

            try (InputStream in = file.getInputStream()) {
                response = (Map<String, Object>) plugin.getClass().getMethod("saveFile", String.class, InputStream.class).invoke(plugin, file.getSubmittedFileName(),in);
            }
            return ResponseEntity.ok(response);
        } catch (Exception e) {
            result.put("success", false);
            result.put("error", e.getMessage());
            return ResponseEntity.ok(result);
        }
    }
}

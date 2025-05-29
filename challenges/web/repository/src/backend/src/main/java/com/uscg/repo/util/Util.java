package com.uscg.repo.util;

import java.io.File;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.Map;
import java.util.List;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.uscg.repo.model.Repo;

public class Util {

    public static Object loadPlugin(Repo r) throws Exception {
        String pluginRoot = "./plugins/storage";
        String pluginClass = r.type;
        File pluginJarFile = new File(pluginRoot, pluginClass + ".jar");

        if (!pluginJarFile.exists()) throw new ClassNotFoundException("Plugin JAR not found");

        URLClassLoader loader = new URLClassLoader(
        new URL[]{pluginJarFile.toURI().toURL()},
        Thread.currentThread().getContextClassLoader()
        );

        Class<?> cls = loader.loadClass(pluginClass);
        Object instance = cls.getDeclaredConstructor().newInstance();

        ObjectMapper mapper = new ObjectMapper();
        Map<String, Object> config = mapper.readValue(r.config, Map.class);

        if (r.type == "LocalStorage") {
            cls.getMethod("configure", Map.class).invoke(instance, Map.of("path","./data/"+r.slug));
        } else {
            cls.getMethod("configure", Map.class).invoke(instance, config);
        }
        
        return instance;
    }

    public static String slugify(String input) {
        return input.toLowerCase()
                    .replaceAll("[^a-z0-9]+", "-")
                    .replaceAll("^-|-$", "");
    }

    public static boolean isAllowedMime(String mime){

        List<String> allowedTypes = List.of(
                "text/plain", "application/json",
                "image/png", "image/jpeg", "image/gif",
                "application/xml", "text/html", "text/csv"
            );

        if (!allowedTypes.contains(mime)) {
            return false;
        }

        return true;
    }

    public static boolean isBinaryMime(String mime){
        List<String> binaryTypes = List.of(
                "image/png", "image/jpeg", "image/gif"
            );
        if (!binaryTypes.contains(mime)) {
            return false;
        }
        return true;
    }
}

// src/main/java/com/uscg/repo/RepoManagerApplication.java
package com.uscg.repo;

import com.uscg.repo.model.Repo;
import com.uscg.repo.repository.RepoRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.CommandLineRunner;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.context.annotation.Bean;
import com.fasterxml.jackson.databind.ObjectMapper;

import java.io.File;
import java.io.FileWriter;
import java.util.Map;

@SpringBootApplication
public class RepoManagerApplication {
    public static void main(String[] args) {
        SpringApplication.run(RepoManagerApplication.class, args);
    }

    @Bean
    CommandLineRunner init(RepoRepository repoRepository) {
        return args -> {
            if (repoRepository.findByName("default") == null) {

                File tempDir = new File("data/default");
                tempDir.mkdirs();

                ObjectMapper mapper = new ObjectMapper();
                Repo r = new Repo();
                r.name = "default";
                r.slug = "default";
                r.type = "LocalStorage";
                r.config = mapper.writeValueAsString(Map.of("path", tempDir.getAbsolutePath()));
                repoRepository.save(r);
            }
        };
    }
}

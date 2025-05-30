package com.uscg.repo.repository;

import com.uscg.repo.model.Repo;
import org.springframework.data.jpa.repository.JpaRepository;

public interface RepoRepository extends JpaRepository<Repo, Long> {
    Repo findByName(String name);
    Repo findBySlug(String slug);
}
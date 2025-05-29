package com.uscg.repo.model;

import jakarta.persistence.*;

@Entity
public class Repo {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    public Long id;
    public String name;
    public String slug;
    public String type;
    
    @Column(length = 4096)
    public String config;
}
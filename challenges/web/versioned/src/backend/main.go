// main.go
package main

import (
	"crypto/rand"
	"database/sql"
	"encoding/base64"
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"time"

	"github.com/golang-jwt/jwt/v5"
	_ "github.com/mattn/go-sqlite3"
	"golang.org/x/crypto/bcrypt"
)

var db *sql.DB
var jwtSecret string

func generateRandomString(n int) string {
	b := make([]byte, n)
	_, err := rand.Read(b)
	if err != nil {
		log.Fatal(err)
	}
	return base64.URLEncoding.EncodeToString(b)[:n]
}

func createUserHandler(w http.ResponseWriter, r *http.Request) {
	var req struct {
		Username string `json:"username"`
		Password string `json:"password"`
	}
	json.NewDecoder(r.Body).Decode(&req)
	req.Username = strings.ToLower(req.Username)
	folder := generateRandomString(12)
	filename := "scratchpad.txt"

	hash, _ := bcrypt.GenerateFromPassword([]byte(req.Password), bcrypt.DefaultCost)
	_, err := db.Exec("INSERT INTO users (username, password, folder, filename) VALUES (?, ?, ?, ?)", req.Username, hash, folder, filename)
	if err != nil {
		http.Error(w, "Username taken", 400)
		return
	}

	os.MkdirAll(filepath.Join("data", folder), 0755)
	os.WriteFile(filepath.Join("data", folder, filename), []byte(""), 0644)

	cmd := exec.Command("git", "init")
	cmd.Dir = filepath.Join("data", folder)
	cmd.Run()

	cmd = exec.Command("git", "add", filename)
	cmd.Dir = filepath.Join("data", folder)
	cmd.Run()

	cmd = exec.Command("git", "commit", "-m", "Initial commit")
	cmd.Dir = filepath.Join("data", folder)
	cmd.Run()

	token := jwt.NewWithClaims(jwt.SigningMethodHS256, jwt.MapClaims{
		"username": req.Username,
		"folder":   folder,
		"filename": filename,
		"exp":      time.Now().Add(time.Hour * 24).Unix(),
	})
	tokenStr, _ := token.SignedString([]byte(jwtSecret))
	json.NewEncoder(w).Encode(map[string]string{"token": tokenStr})
}

func loginHandler(w http.ResponseWriter, r *http.Request) {
	var req struct {
		Username string `json:"username"`
		Password string `json:"password"`
	}
	json.NewDecoder(r.Body).Decode(&req)
	req.Username = strings.ToLower(req.Username)

	row := db.QueryRow("SELECT password, folder, filename FROM users WHERE username = ?", req.Username)
	var hash, folder, filename string
	if err := row.Scan(&hash, &folder, &filename); err != nil {
		http.Error(w, "Invalid login", 400)
		return
	}
	if bcrypt.CompareHashAndPassword([]byte(hash), []byte(req.Password)) != nil {
		http.Error(w, "Invalid login", 400)
		return
	}

	token := jwt.NewWithClaims(jwt.SigningMethodHS256, jwt.MapClaims{
		"username": req.Username,
		"folder":   folder,
		"filename": filename,
		"exp":      time.Now().Add(time.Hour * 24).Unix(),
	})
	tokenStr, _ := token.SignedString([]byte(jwtSecret))
	json.NewEncoder(w).Encode(map[string]string{"token": tokenStr})
}

func usernameCheckHandler(w http.ResponseWriter, r *http.Request) {
	username := strings.ToLower(r.URL.Query().Get("username"))
	row := db.QueryRow("SELECT 1 FROM users WHERE username = ?", username)
	var dummy int
	err := row.Scan(&dummy)
	json.NewEncoder(w).Encode(map[string]bool{"good": err == sql.ErrNoRows})
}

func getClaims(r *http.Request) (jwt.MapClaims, error) {
	tokenStr := strings.TrimPrefix(r.Header.Get("Authorization"), "Bearer ")
	token, err := jwt.Parse(tokenStr, func(token *jwt.Token) (interface{}, error) {
		return []byte(jwtSecret), nil
	})
	if err != nil || !token.Valid {
		return nil, fmt.Errorf("invalid token")
	}
	return token.Claims.(jwt.MapClaims), nil
}

func getFileHandler(w http.ResponseWriter, r *http.Request) {
	claims, err := getClaims(r)
	if err != nil {
		http.Error(w, "Unauthorized", 401)
		return
	}
	folder := claims["folder"].(string)
	filename := claims["filename"].(string)
	data, err := os.ReadFile(filepath.Join("data", folder, filename))
	if err != nil {
		http.Error(w, "Failed to read file", 500)
		return
	}
	w.Write(data)
}

func saveFileHandler(w http.ResponseWriter, r *http.Request) {
	claims, err := getClaims(r)
	if err != nil {
		http.Error(w, "Unauthorized", 401)
		return
	}
	folder := claims["folder"].(string)
	filename := claims["filename"].(string)

	var req struct {
		Content string `json:"content"`
	}
	json.NewDecoder(r.Body).Decode(&req)

	path := filepath.Join("data", folder, filename)
	os.WriteFile(path, []byte(req.Content), 0644)

	exec.Command("git", "add", filename).Dir = filepath.Join("data", folder)
	exec.Command("git", "commit", "-m", time.Now().Format(time.RFC3339)).Dir = filepath.Join("data", folder)
	w.WriteHeader(http.StatusOK)
}

func revisionsHandler(w http.ResponseWriter, r *http.Request) {
	claims, err := getClaims(r)
	if err != nil {
		http.Error(w, "Unauthorized", 401)
		return
	}
	folder := claims["folder"].(string)

	cmd := exec.Command("git", "log", "--pretty=format:%H %cI")
	cmd.Dir = filepath.Join("data", folder)
	out, err := cmd.Output()
	if err != nil {
		http.Error(w, "Failed to get log", 500)
		return
	}
	lines := strings.Split(string(out), "\n")
	var revs []map[string]string
	for _, line := range lines {
		parts := strings.SplitN(line, " ", 2)
		if len(parts) == 2 {
			revs = append(revs, map[string]string{"hash": parts[0], "date": parts[1]})
		}
	}
	json.NewEncoder(w).Encode(revs)
}

func revisionContentHandler(w http.ResponseWriter, r *http.Request) {
	claims, err := getClaims(r)
	if err != nil {
		http.Error(w, "Unauthorized", 401)
		return
	}
	folder := claims["folder"].(string)
	filename := claims["filename"].(string)
	hash := strings.TrimPrefix(r.URL.Path, "/api/revision/")

	cmd := exec.Command("git", "show", fmt.Sprintf("%s:%s", hash, filename))
	cmd.Dir = filepath.Join("data", folder)
	out, err := cmd.Output()
	if err != nil {
		http.Error(w, "Failed to load revision", 500)
		return
	}
	json.NewEncoder(w).Encode(map[string]string{"content": string(out)})
}

func main() {
	jwtSecret = generateRandomString(32)
	db, _ = sql.Open("sqlite3", "file:ctf.db?_foreign_keys=on")
	db.Exec("CREATE TABLE IF NOT EXISTS users (username TEXT PRIMARY KEY, password TEXT, folder TEXT, filename TEXT)")

	http.HandleFunc("/api/register", createUserHandler)
	http.HandleFunc("/api/login", loginHandler)
	http.HandleFunc("/api/username_check", usernameCheckHandler)
	http.HandleFunc("/api/file", getFileHandler)
	http.HandleFunc("/api/save", saveFileHandler)
	http.HandleFunc("/api/revisions", revisionsHandler)
	http.HandleFunc("/api/revision/", revisionContentHandler)

	log.Println("Server running on :8080")
	http.ListenAndServe(":8080", nil)
}

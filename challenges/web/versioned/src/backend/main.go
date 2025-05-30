package main

import (
	"crypto/rand"
	"database/sql"
	"fmt"
	"net/http"
	"os"
	"os/exec"
	"path/filepath"
	"time"

	"github.com/gin-gonic/gin"
	"github.com/golang-jwt/jwt/v5"
	"github.com/google/uuid"
	_ "github.com/mattn/go-sqlite3"
	"golang.org/x/crypto/bcrypt"
)

var db *sql.DB
var jwtSecret = generateRandomSecret(32)

func generateRandomSecret(length int) []byte {
	secret := make([]byte, length)
	_, err := rand.Read(secret)
	if err != nil {
		panic("Failed to generate random secret: " + err.Error())
	}
	return secret
}

func main() {
	var err error
	db, err = sql.Open("sqlite3", "./users.db")
	if err != nil {
		panic(err)
	}

	initDB()

	r := gin.Default()
	r.POST("/api/register", register)
	r.POST("/api/login", login)
	r.GET("/api/files", authMiddleware, listFiles)
	r.GET("/api/files/:name", authMiddleware, getFile)
	r.POST("/api/save", authMiddleware, saveFile)

	r.Run(":8080")
}

func initDB() {
	_, err := db.Exec(`CREATE TABLE IF NOT EXISTS users (
		id INTEGER PRIMARY KEY AUTOINCREMENT,
		username TEXT UNIQUE,
		password TEXT,
		folder TEXT
	);`)
	if err != nil {
		panic(err)
	}
}

func register(c *gin.Context) {
	var creds struct {
		Username string `json:"username"`
		Password string `json:"password"`
	}
	if err := c.BindJSON(&creds); err != nil {
		c.String(http.StatusBadRequest, "Invalid input")
		return
	}

	id := uuid.New().String()
	folderPath := filepath.Join("/folders", id)

	if err := os.MkdirAll(folderPath, 0755); err != nil {
		c.String(http.StatusInternalServerError, "Failed to create folder")
		return
	}
	cmd := exec.Command("git", "init")
	cmd.Dir = folderPath
	cmd.Run()

	hashedPassword, err := bcrypt.GenerateFromPassword([]byte(creds.Password), bcrypt.DefaultCost)
	if err != nil {
		c.String(http.StatusInternalServerError, "Failed to hash password")
		return
	}

	_, err = db.Exec("INSERT INTO users (username, password, folder) VALUES (?, ?, ?)",
		creds.Username, hashedPassword, folderPath)
	if err != nil {
		c.String(http.StatusConflict, "User already exists")
		return
	}

	token, err := generateJWT(creds.Username)
	if err != nil {
		c.String(http.StatusInternalServerError, "Failed to generate token")
		return
	}
	c.JSON(http.StatusOK, gin.H{"token": token})
}

func login(c *gin.Context) {
	var creds struct {
		Username string `json:"username"`
		Password string `json:"password"`
	}
	if err := c.BindJSON(&creds); err != nil {
		c.String(http.StatusBadRequest, "Invalid input")
		return
	}
	var storedPassword string
	err := db.QueryRow("SELECT password FROM users WHERE username = ?", creds.Username).Scan(&storedPassword)
	if err != nil || bcrypt.CompareHashAndPassword([]byte(storedPassword), []byte(creds.Password)) != nil {
		c.String(http.StatusUnauthorized, "Invalid credentials")
		return
	}

	token, err := generateJWT(creds.Username)
	if err != nil {
		c.String(http.StatusInternalServerError, "Failed to generate token")
		return
	}
	c.JSON(http.StatusOK, gin.H{"token": token})
}

func generateJWT(username string) (string, error) {
	claims := jwt.MapClaims{
		"username": username,
		"exp":      time.Now().Add(time.Hour).Unix(),
	}
	token := jwt.NewWithClaims(jwt.SigningMethodHS256, claims)
	return token.SignedString(jwtSecret)
}

func authMiddleware(c *gin.Context) {
	authHeader := c.GetHeader("Authorization")
	if authHeader == "" || len(authHeader) < 8 || authHeader[:7] != "Bearer " {
		c.AbortWithStatus(http.StatusUnauthorized)
		return
	}
	tokenString := authHeader[7:]

	token, _ := jwt.Parse(tokenString, func(token *jwt.Token) (interface{}, error) {
		if _, ok := token.Method.(*jwt.SigningMethodHMAC); !ok {
			return nil, fmt.Errorf("unexpected signing method")
		}
		return jwtSecret, nil
	})

	if claims, ok := token.Claims.(jwt.MapClaims); ok && token.Valid {
		c.Set("username", claims["username"].(string))
		c.Next()
	} else {
		c.AbortWithStatus(http.StatusUnauthorized)
	}
}

func getFolderPath(username string) (string, error) {
	var folder string
	err := db.QueryRow("SELECT folder FROM users WHERE username = ?", username).Scan(&folder)
	return folder, err
}

func listFiles(c *gin.Context) {
	username := c.GetString("username")
	folder, err := getFolderPath(username)
	if err != nil {
		c.String(http.StatusInternalServerError, "Folder not found")
		return
	}
	files, err := os.ReadDir(folder)
	if err != nil {
		c.String(http.StatusInternalServerError, "Cannot read folder")
		return
	}
	names := []string{}
	for _, f := range files {
		if !f.IsDir() && filepath.Ext(f.Name()) == ".txt" {
			names = append(names, f.Name())
		}
	}
	c.JSON(http.StatusOK, names)
}

func getFile(c *gin.Context) {
	username := c.GetString("username")
	filename := c.Param("name")
	folder, err := getFolderPath(username)
	if err != nil {
		c.String(http.StatusInternalServerError, "Folder not found")
		return
	}
	path := filepath.Join(folder, filepath.Clean(filename))
	data, err := os.ReadFile(path)
	if err != nil {
		c.String(http.StatusNotFound, "File not found")
		return
	}
	c.String(http.StatusOK, string(data))
}

func saveFile(c *gin.Context) {
	username := c.GetString("username")
	var input struct {
		Filename string `json:"filename"`
		Content  string `json:"content"`
	}
	if err := c.BindJSON(&input); err != nil {
		c.String(http.StatusBadRequest, "Invalid input")
		return
	}
	folder, err := getFolderPath(username)
	if err != nil {
		c.String(http.StatusInternalServerError, "Folder not found")
		return
	}
	path := filepath.Join(folder, filepath.Clean(input.Filename))
	if err := os.WriteFile(path, []byte(input.Content), 0644); err != nil {
		c.String(http.StatusInternalServerError, "Write failed")
		return
	}
	exec.Command("git", "-C", folder, "add", input.Filename).Run()
	exec.Command("git", "-C", folder, "commit", "-m", "Update "+input.Filename).Run()
	c.String(http.StatusOK, "Saved")
}

from requests import get, post

BASE_URL = "http://localhost:8080"

def get_path(path):
    headers = {"User-Agent": "NinjaNote 13.37"}
    return get(BASE_URL + path, headers=headers)

def post_path(path, data):
    headers = {"User-Agent": "NinjaNote 13.37"}
    return post(BASE_URL + path, headers=headers, json = data)

print("Welcome to NinjaNote!\n\nVersion: 13.37\n\n")

while True:
    primary_selection = ""
    while primary_selection not in ["1", "2", "3"]:
        print("What do you want to do?\n[1] Create note\n[2] Search note\n[3] Exit")
        primary_selection = input("Your selection: ")
        if primary_selection not in ["1", "2", "3"]:
            print("Invalid selection. Try again.")
        print("\n")
    if primary_selection == "1":
        print("Creating new note")
        title = input("Title: ")
        # Hopefully this should be enough?
        note_content = input("Note content: ").replace("{", "").replace("}", "")
        res = post_path("/api/submit", {"title": title, "content": note_content}).json()
        if 'note_id' in res:
            print("Success! Your note ID is: " + res['note_id'])
        else:
            print("Error in posting your note: ", res)
    elif primary_selection == "2":
        print("Retrieving note")
        note_id = input("Note ID to retrieve: ")
        print(get_path("/api/notes/" + note_id).text)
    else:
        break
    print("\n")

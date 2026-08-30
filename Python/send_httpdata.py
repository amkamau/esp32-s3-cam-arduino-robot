import requests

url = "http://192.168.0.50/api/arduino/command"

command = "1111000281501"

response = requests.post(url, data=command)

print(response.status_code)
print(response.text)

# MongooseServiceDemo
Mongoose embedded webserver running as a windows server

1. copy the folder `web_root` to the c drive
2. run powershell as admin and cd to `c:/web_root`
3. run `AISToCoT -install`
<img width="569" height="91" alt="image" src="https://github.com/user-attachments/assets/fd705dd3-b1e1-4b21-b740-b22b672a0e7c" />

4. To remove the service run `AISToCoT -remove`
<img width="578" height="90" alt="image" src="https://github.com/user-attachments/assets/6b9bd597-b0ff-4235-b99b-a305b73e76c6" />


5. Then open the windows services panel and find AIStoCOT Service
5a. Right click and run
5b. Optionally set it to auto start: `Properties -> Startup type -> Automatic`

<img width="1230" height="869" alt="image" src="https://github.com/user-attachments/assets/f92e766a-4e7c-4db6-8b9a-34a018f646ba" />



6. You can surf to `http://localhost:8500/webui.html`
to see the status

<img width="1067" height="939" alt="image" src="https://github.com/user-attachments/assets/c9a34c94-04e1-4371-bd78-baa5625b1691" />


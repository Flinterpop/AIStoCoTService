# MongooseServiceDemo
Mongoose embedded webserver running as a windows server

1. copy the folder `web_root` to the c drive
2. run powershell as admin and cd to `c:/web_root`

> [!NOTE]  
>  Windows Services require absolute paths and the easiest way to configure this is to use an application folder in the root of the C-drive.

4. run `AISToCoT.exe -install`
<img width="569" height="91" alt="image" src="https://github.com/user-attachments/assets/fd705dd3-b1e1-4b21-b740-b22b672a0e7c" />

5. To remove the service run `AISToCoT.exe -remove`
<img width="578" height="90" alt="image" src="https://github.com/user-attachments/assets/6b9bd597-b0ff-4235-b99b-a305b73e76c6" />


6. Then open the windows services panel and find AIStoCOT Service
5a. Right click and run

5b. Optionally set it to auto start: `Properties -> Startup type -> Automatic`

<img width="1230" height="869" alt="image" src="https://github.com/user-attachments/assets/f92e766a-4e7c-4db6-8b9a-34a018f646ba" />



6. You can surf to `http://localhost:8500/webui.html`
to see the status

<img width="1067" height="939" alt="image" src="https://github.com/user-attachments/assets/c9a34c94-04e1-4371-bd78-baa5625b1691" />

---
### What it does
The service conencts to a COM Port defined in `web_root/AISToCoT.ini`
for example COM1 at baud 38400 (typical for a dAISy AIS Receiver).
It parses the AIS message and sends a CoT message on the multicast group and port also defined in `web_root/AISToCoT.ini`
The default is 239.2.3.1: 6969

Example AIS NMEA string:
`!AIVDM,1,1,,B,34eHKFm000o:pgBKmgq>48h40DTJ,0*6B`

Result on WinTAK:
<img width="799" height="655" alt="Capture" src="https://github.com/user-attachments/assets/be51f4c2-a409-4591-b6f3-2b2bfc7db981" />

The CoT Message:
<img width="270" height="204" alt="Capture2" src="https://github.com/user-attachments/assets/b78da5ac-27d4-4f63-9139-67199c0249b5" />



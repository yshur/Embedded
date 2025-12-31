/**** ESP32 File Manager for SPIFFS storage
     Written by: Amit Resh, May 2025

    Based on ideas from My Circuits 2022, based on code of David Bird 2018 
        
    THE SOFTWARE IS PROVIDED "AS IS" FOR PRIVATE USE ONLY, IT IS NOT FOR COMMERCIAL USE IN WHOLE OR PART OR CONCEPT. FOR PERSONAL USE IT IS SUPPLIED WITHOUT WARRANTY 
    OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
****/

#include <WiFi.h>              //Built-in
#include <WebServer.h>
#include <SPIFFS.h>

#include "SPIFFDir.html"          //SPIFFS Directory Listing HTML form
#include "UploadForm.html"        //Filel Upload Form

#define   USE_SOFTAP

WebServer server(80);


/*********  FUNCTIONS  **********/

String loadPageWithMessage(const String& msg) {
  String html {upload_File_html};
  html.replace("{{message}}", msg);
  return html;
}

void File_Upload(void)
{
  server.send(200, "text/html", loadPageWithMessage(""));   //Display File Upload w/o ErrorMsg
}

//  ======  Upload a new file to SPIFFS ==============================================

File UploadFp;
bool uploadFail;

void handleFileUpload()
{ 
  HTTPUpload& uploadfile = server.upload(); //See https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer/srcv
                                            //For further information on 'status' structure, there are other reasons such as a failed transfer that could be used
  if(uploadfile.status == UPLOAD_FILE_START)
  {
    String filename = uploadfile.filename;
    uploadFail=false;
    if(!filename.startsWith("/")) filename = "/"+filename;
    Serial.print("Upload File Name: "); Serial.println(filename);
    Serial.printf("SPIFFS Usage: %d of %d (Remaining: %d) [bytes]\n", SPIFFS.usedBytes(), SPIFFS.totalBytes(), SPIFFS.totalBytes()-SPIFFS.usedBytes());
    if (SPIFFS.exists(filename))
      SPIFFS.remove(filename);

    UploadFp = SPIFFS.open(filename, FILE_WRITE);                           //Open file on SPIFFS
    if (!UploadFp || !UploadFp.seek(0)) {
      uploadFail = true;
      server.send(200, "text/html", loadPageWithMessage("Couldn't create "+uploadfile.filename+" in SPIFFS"));
      Serial.println("\nFile POST endded");
    }
  }
  else if (uploadfile.status == UPLOAD_FILE_WRITE)
  {
    size_t sz;
    if(UploadFp && !uploadFail) {
      sz=UploadFp.write(uploadfile.buf, uploadfile.currentSize);    //Write the received bytes to the file
      Serial.print('.');
      if (sz != uploadfile.currentSize) {
        uploadFail = true;
        UploadFp.close();
        server.send(200, "text/html", loadPageWithMessage("Write error for "+uploadfile.filename+" in SPIFFS"));
        Serial.println("\nFile POST endded");
      }
    }
  } 
  else if (uploadfile.status == UPLOAD_FILE_END)
  {
    Serial.println("\nFile POST endded");
    if(UploadFp && !uploadFail)          //If the file was successfully created
    {                                    
      UploadFp.close();   //Close the file again
      Serial.print("Upload Size: "); Serial.println(uploadfile.totalSize);
      server.send(200,"text/html",loadPageWithMessage("File "+uploadfile.filename+" Successfully saved to SPIFFS"));
    } 
    // else
    // {
    //   server.send(200, "text/html", loadPageWithMessage("Could Not Create File "+uploadfile.filename+" in SPIFFS"));
    // }
  }
}

String listFilesHTML() {
  String html = SPIFF_FileList_html;

  File root = SPIFFS.open("/");
  if (!root) return "<p>⚠️ Failed to open SPIFFS directory.</p>";

  File file = root.openNextFile();
  int count = 0;

  while (file) {
    String fileName = file.name();
    size_t fileSize = file.size();
    html += "<div class='file-item'><div class='file-name'>" + 
            String(++count) + ". <strong>" + fileName + "</strong> — " + String(fileSize) + " bytes</div>";
    html += "<form method='POST' action='/delete' style='margin:0;'>"
            "<input type='hidden' name='filename' value='" + fileName + "'>"
            "<button class='delete-button' type='submit'>🗑 Delete</button></form></div>";
    file = root.openNextFile();
  }

  if (count == 0) {
    html += "<div class='file-item'>No files found.</div>";
  }

  html += SPIFF_ListTermin_html;

  return html;
}

void handleFileList() {
  String html = ""; //"<!DOCTYPE html><html><head><title>SPIFFS Files Manager</title></head><body>";
  html += listFilesHTML();
  html += "</body></html>";
  server.send(200, "text/html", html);
}

/*********  SETUP  **********/
void setup(void)
{  
  Serial.begin(115200);
  
  //SPIFFS
  if(!SPIFFS.begin(true)){
    if (!SPIFFS.begin(true)) {
      Serial.println("An Error has occurred while mounting SPIFFS");
      for(Serial.print("Stopped ...");;delay(1000))
        ;
    }
  }
  Serial.println("SPIFFS Initialized...");

  #ifdef USE_SOFTAP
    //Server Initialization 
    #define   SSID  "Yair-WebUpload"   //Web-Server SSID

    WiFi.softAP(SSID, "12345678"); //Network and password for the access point genereted by ESP32
  #else
    const char* ssid = "Galaxy M54 5G 44AD";  //Your WiFi SSID
    const char* password = "0545456527";

    WiFi.begin(ssid, password);
    while(WiFi.status() != WL_CONNECTED) { 
      delay(500);
      Serial.print(".");
    }
  #endif
    
  /*********  Server Commands  **********/
  server.on("/",         handleFileList);
  server.on("/upload",   File_Upload);
  server.on("/fupload",  HTTP_POST,[](){ server.send(200);}, handleFileUpload);

  server.on("/delete", HTTP_POST, []() {
    if (server.hasArg("filename")) {
      String filename = server.arg("filename");
      if(!filename.startsWith("/")) filename = "/"+filename;
      if (SPIFFS.exists(filename)) {
        SPIFFS.remove(filename);
        server.sendHeader("Location", "/", true);
        server.send(303);
      } else {
        server.send(404, "text/plain", "File not found");
      }
    } else {
      server.send(400, "text/plain", "Filename not provided");
    }
  });

  server.begin();

  #ifdef USE_SOFTAP  
    Serial.printf("WEB server started ... Connect to SSID: %s and browse to address 192.168.4.1\n", SSID);
  #else
    Serial.print("Connect to Web Server on IP Address: ");
    Serial.println(WiFi.localIP());
  #endif
}

/*********  LOOP  **********/
void loop(void)
{
  server.handleClient(); //Listen for client connections
  delay(1000);
}

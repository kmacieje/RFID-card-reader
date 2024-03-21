var SS = SpreadsheetApp.openById('*Google Sheet ID*');
var timezone = "Europe/Warsaw";
var hours = 0;
var str = "";


function doPost(e) {

  var parsedData;
  var result = {};
  
  try { 
    parsedData = JSON.parse(e.postData.contents);
  } 
  catch(f){
    return ContentService.createTextOutput("Error in parsing request body: " + f.message);
  }
   
  if (parsedData !== undefined){
    var flag = parsedData.format;
    if (flag === undefined){
      flag = 0;
    }
    
    var sheet = SS.getSheetByName(parsedData.sheet_name); // sheet name to publish data to is specified in Arduino code
    var dataArr = parsedData.values.split(","); // creates an array of the values to publish 
         
    var Curr_Date = Utilities.formatDate(new Date(), timezone, "dd/MM/yyyy"); // gets the current date
    var Curr_Time = Utilities.formatDate(new Date(), timezone, "hh:mm:ss a"); // gets the current time

    // comming from Arduino code
    var value0 = dataArr [0];  // Name - who
    var value1 = dataArr [1];  // ID - value

  
    var data = sheet.getDataRange().getValues();
    var row_number = 0;
    var time_out = "";

    for(var i = 0; i < data.length ; i++){  // Search first occurrence of student id
      if(data[i][0] == value1){ //data[i][0] i.e. [0]=Column A, Student_id
        row_number = i+1;
        time_out = data[i][4] //time out [4]=Column E
        
        console.log("row number: "+row_number); //print row number
        console.log("time out: "+time_out); //print row number
		break; //go outside the loop
      }
    }

    if(row_number > 0){
      if(time_out === "" && value0 !== "Unknown"){
        sheet.getRange("E"+row_number).setValue(Curr_Time);
        str = "Success"; // string to return back to Arduino serial console
        return ContentService.createTextOutput(str);
      }
    }

    switch (parsedData.command) {
      
      case "insert_row":
         
         sheet.insertRows(2); // insert full row directly below header text
         
         sheet.getRange('A2').setValue(value1);     // publish ID to cell A2
         sheet.getRange('B2').setValue(value0);     // publish NAME cell B2
         sheet.getRange('C2').setValue(Curr_Date);  // publish DATE to cell C2
         sheet.getRange('D2').setValue(Curr_Time);  // publish TIME IN to cell D2
         
         str = "Success"; // string to return back to Arduino serial console
         SpreadsheetApp.flush();
         break;
            
    }
    
    return ContentService.createTextOutput(str);
  } 
  
  else {
    return ContentService.createTextOutput("Error! Request body empty or in incorrect format.");
  }
}
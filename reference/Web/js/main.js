var mqtt;
var reconnectTimeout = 2000;
var host="ngoinhaiot.com"; 
var port=2222;
// var lwt = new Paho.MQTT.Message("false");
// lwt.destinationName = "hoangtoancsgl/myboardisonline";
// lwt.qos = 0;
// lwt.retained = false;

function onConnect()
{
    // Once a connection has been made, make a subscription and send a message.
    console.log("Connected to ngoinhaiot.com");
    mqtt.subscribe("hoangtoancsgl/#");
    message = new Paho.MQTT.Message("Hello from web!");
    message.destinationName = "hoangtoancsgl/myboardisonline";
    mqtt.send(message);
}

function CallBack(mes) 
{
    if(mes.destinationName.indexOf("ESP")>=0)
    {
        console.log("Topic:     " + mes.destinationName );
        console.log("Message Arrived: " + mes.payloadString );
    }
}

function MQTTconnect()
{
    console.log("connecting to "+ host +" "+ port);
    mqtt = new Paho.MQTT.Client(host,port,"clientjs");
    console.log("connecting to "+ host);
    var options = {
        onSuccess: onConnect,
        //willMessage : lwt,
        userName : "hoangtoancsgl",
        password : "850B3436127D4E73",
    };

    //Connect
    mqtt.connect(options); 
    //Set Calback
    mqtt.onMessageArrived = CallBack;
    
}

function Button1()
{
    message = new Paho.MQTT.Message("Button 1 pressed!");
    message.destinationName = "hoangtoancsgl/button1";
    mqtt.send(message);
    alert("Đã bật đèn!")
}

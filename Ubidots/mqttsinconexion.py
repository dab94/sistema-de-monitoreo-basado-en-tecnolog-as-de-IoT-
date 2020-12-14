import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish


def on_connect(client, userdata, flags, rc):
    print ("Connected with result code"+str(rc))
    client.subscribe("#",0)

    
def on_message(client,userdata,msg):
 
    print (msg.topic+" "+str(msg.payload))
    publish.single(msg.topic,msg.payload,0,hostname='things.ubidots.com',auth = {'username':""})
    
    
client=mqtt.Client()
client.on_connect=on_connect
client.on_message=on_message
client.connect('localhost',1883,60)
client.loop_forever()

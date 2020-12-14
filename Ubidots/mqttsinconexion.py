import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish

hola='NO'
def on_connect(client, userdata, flags, rc):
    print ("Connected with result code"+str(rc))
    client.subscribe("#",0)

    
def on_message(client,userdata,msg):
    global hola
    print (msg.topic+" "+str(msg.payload))
    publish.single(msg.topic,msg.payload,0,hostname='things.ubidots.com',auth = {'username':"n31rZtaWi1j5umLjF906IJeH9kcff4"})
    #hola=msg.payload
    #print('Aqui', hola)
    
client=mqtt.Client()
client.on_connect=on_connect
client.on_message=on_message
client.connect('localhost',1883,60)
client.loop_forever()

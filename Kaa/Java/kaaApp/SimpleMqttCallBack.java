package kaaApp;


import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttMessage;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.json.JSONException;
import org.json.JSONObject;

public class SimpleMqttCallBack implements MqttCallback {
	public static String salida;
	public static String Topic;
	public static Boolean llego;
	public static ArrayList<String>  variables =new ArrayList<String>();
	public static ArrayList<Integer>  values =new ArrayList<Integer>();



  public void connectionLost(Throwable throwable) {
    System.out.println("Connection to MQTT broker lost!");
  }

  public void messageArrived(String s, MqttMessage mqttMessage) throws Exception {
    salida= new String(mqttMessage.getPayload());
    Topic= s;
    System.out.println(salida);
    System.out.println(Topic);
    mensaje();
    
  }

  public void deliveryComplete(IMqttDeliveryToken iMqttDeliveryToken) {
  }
  
  public static ArrayList<String> var(){
	  return variables;
  
  }
  
  public static ArrayList<Integer> val(){
	  return values;
  
  }
  
  public static String topa(){
	  System.out.println(Topic);
	  return Topic;
  }
  
  public static void mensaje() throws JSONException{
	  JSONObject jsonObj = new JSONObject(salida);
	  Iterator<?> keys = jsonObj.keys();
	  
	  //List<String> variables =new ArrayList<String>();
	  variables.clear();
	  //List<Integer> values =new ArrayList<Integer>();
	  values.clear();

	  while( keys.hasNext() ) {
	      String key = (String) keys.next();
	      variables.add(key);
	      int val = (int) jsonObj.get(key);
	      values.add(val);
	      System.out.println("Key: " + key);
	      /*System.out.println("Value: " + val);*/
	      
	  }
  
  }
}

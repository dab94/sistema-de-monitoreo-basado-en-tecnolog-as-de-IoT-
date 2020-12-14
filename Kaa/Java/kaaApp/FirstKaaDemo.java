package kaaApp;

import org.eclipse.paho.client.mqttv3.MqttException;
import org.kaaproject.kaa.client.DesktopKaaPlatformContext;
import org.kaaproject.kaa.client.Kaa;
import org.kaaproject.kaa.client.KaaClient;
import org.kaaproject.kaa.client.SimpleKaaClientStateListener;
import org.kaaproject.kaa.client.configuration.base.ConfigurationListener;
import org.kaaproject.kaa.client.configuration.base.SimpleConfigurationStorage;
import org.kaaproject.kaa.client.logging.strategies.RecordCountLogUploadStrategy;
import org.kaaproject.kaa.schema.newapp.DataCollection;
import org.kaaproject.kaa.schema.newapp.Configuration;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import java.io.IOException;
import java.util.ArrayList;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;
 
/**
 * Class implement functionality for First Kaa application. Application send temperature data
 * from the Kaa endpoint with required configured sampling period
 */
public class FirstKaaDemo {
 
    private static final long DEFAULT_START_DELAY = 1000L;
 
    private static final Logger LOG = LoggerFactory.getLogger(FirstKaaDemo.class);
 
    private static KaaClient kaaClient;
    
    private static Subscriber sub;
    
    private static SimpleMqttCallBack callb;
    
 
    private static ScheduledFuture<?> scheduledFuture;
    private static ScheduledExecutorService scheduledExecutorService;
 
    public static void main(String[] args) {
        LOG.info(FirstKaaDemo.class.getSimpleName() + " app starting!");
 
        scheduledExecutorService = Executors.newScheduledThreadPool(1);
        
        //Create the Kaa desktop context for the application.
        DesktopKaaPlatformContext desktopKaaPlatformContext = new DesktopKaaPlatformContext();
 
        /*
         * Create a Kaa client and add a listener which displays the Kaa client
         * configuration as soon as the Kaa client is started.
         */
        kaaClient = Kaa.newClient(desktopKaaPlatformContext, new FirstKaaClientStateListener(), true);
 
        /*
         *  Used by log collector on each adding of the new log record in order to check whether to send logs to server.
         *  Start log upload when there is at least one record in storage.
         */
        RecordCountLogUploadStrategy strategy = new RecordCountLogUploadStrategy(1);
        strategy.setMaxParallelUploads(1);
        kaaClient.setLogUploadStrategy(strategy);
 
        /*
         * Persist configuration in a local storage to avoid downloading it each
         * time the Kaa client is started.
         */
        kaaClient.setConfigurationStorage(new SimpleConfigurationStorage(desktopKaaPlatformContext, "saved_config.cfg"));
 
        kaaClient.addConfigurationListener(new ConfigurationListener() {
            @Override
            public void onConfigurationUpdate(Configuration configuration) {
                LOG.info("Received configuration data. New sample period: {}", configuration.getSamplePeriod());
                onChangedConfiguration(TimeUnit.SECONDS.toMillis(configuration.getSamplePeriod()));
            }
        });
  
        //Start the Kaa client and connect it to the Kaa server.
        kaaClient.start();
 
        LOG.info("--= Press any key to exit =--");
        try {
            System.in.read();
        } catch (IOException e) {
            LOG.error("IOException has occurred: {}", e.getMessage());
        }
        LOG.info("Stopping...");
        scheduledExecutorService.shutdown();
        kaaClient.stop();
    }
 
    /*
     * Method, that emulate getting temperature from real sensor.
     * Retrieves random temperature.
     */
    private static String getTemperatureRand() {
        String palabra="hola";
        return palabra;
    }
 
    private static void onKaaStarted(long time) {
        if (time <= 0) {
            LOG.error("Wrong time is used. Please, check your configuration!");
            kaaClient.stop();
            System.exit(0);
        }
 
        scheduledFuture = scheduledExecutorService.scheduleAtFixedRate(
                new Runnable() {
                    @Override
                    public void run() {
						try {
							sub.main();
						} catch (MqttException e) {
							// TODO Auto-generated catch block
							e.printStackTrace();
						}
						System.out.println("hola2");
						ArrayList<Integer> valores = callb.val();
						ArrayList<String> variables = callb.var();
						
                    	String firstKey = variables.get(0);
                    	String secondKey = variables.get(1);
                    	String thirdKey = variables.get(2);
                    	
                    	Integer firstValue = valores.get(0);
                    	Integer secondValue = valores.get(1);
                    	Integer thirdValue = valores.get(2);
                    	
                    	String topica = SimpleMqttCallBack.topa();
                    	
						//String Broker = "maquina";
                    	//System.out.println(Broker);
                        /*String Broker = getTemperatureRand();*/
                    	System.out.println("hola1");
                        kaaClient.addLogRecord(new DataCollection(firstValue,secondValue,thirdValue,topica));
                        System.out.println("hola");
                        LOG.info("Sampled Temperature: {}", firstValue);
                    }
                }, 0, time, TimeUnit.MILLISECONDS);
    }
 
    private static void onChangedConfiguration(long time) {
        if (time == 0) {
            time = DEFAULT_START_DELAY;
        }
        scheduledFuture.cancel(false);
 
        scheduledFuture = scheduledExecutorService.scheduleAtFixedRate(
                new Runnable() {
                    @Override
                    public void run() {
                        Integer Broker = 2;
                        kaaClient.addLogRecord(new DataCollection(Broker, Broker, Broker,"Hola"));
                        LOG.info("Sampled Temperature: {}", Broker);
                    }
                }, 0, time, TimeUnit.MILLISECONDS);
    }
 
    private static class FirstKaaClientStateListener extends SimpleKaaClientStateListener {
 
        @Override
        public void onStarted() {
            super.onStarted();
            LOG.info("Kaa client started");
            Configuration configuration = kaaClient.getConfiguration();
            LOG.info("Default sample period: {}", configuration.getSamplePeriod());
            onKaaStarted(TimeUnit.SECONDS.toMillis(configuration.getSamplePeriod()));
        }
 
        @Override
        public void onStopped() {
            super.onStopped();
            LOG.info("Kaa client stopped");
        }
    }
}
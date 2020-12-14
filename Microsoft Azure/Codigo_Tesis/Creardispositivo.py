CONNECTION_STRING = "e"
DEVICE_ID = "d"


def print_device_info(title, iothub_device):
    print ( title + ":" )
    print ( "iothubDevice.deviceId                    = {0}".format(iothub_device.deviceId) )
    print ( "iothubDevice.primaryKey                  = {0}".format(iothub_device.primaryKey) )
    print ( "iothubDevice.secondaryKey                = {0}".format(iothub_device.secondaryKey) )
    print ( "iothubDevice.generationId                = {0}".format(iothub_device.generationId) )


def iothub_registrymanager_sample_run():
    try:
        # RegistryManager
        iothub_registry_manager = IoTHubRegistryManager(CONNECTION_STRING)

        # CreateDevice
	primary_key = "3"
	secondary_key = "4"
        auth_method = IoTHubRegistryManagerAuthMethod.SHARED_PRIVATE_KEY
        new_device = iothub_registry_manager.create_device(DEVICE_ID, primary_key, secondary_key, auth_method)

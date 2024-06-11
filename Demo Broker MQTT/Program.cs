namespace Demo_Broker_MQTT;

internal class Program
{
    static async Task Main()
    {
        await ServerMQTT.BuilderBroker();
    }
}

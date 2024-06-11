namespace Demo_Broker_MQTT.Modelos
{
    public class RequestClient(string message, string idCliend, string topic)
    {
        public string Message = message;
        public string IdClient = idCliend;
        public string Topic = topic;
    }
}

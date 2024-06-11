using Demo_Broker_MQTT.Logic;
using Demo_Broker_MQTT.Modelos;
using MQTTnet.Protocol;
using MQTTnet.Server;
using System.Text;

internal class ServerMQTT
{
    #region Metodo de cracion del servidor

    ///<summary>
    ///Creation and management the server or broker MQTT
    ///</summary>
    public async static Task BuilderBroker()
    {

        MqttServer _serverM;//-------------------------------------------------------------------------------------->Server Instruction
        Services _service = new();//------------------------------------------------------------------------------------------>Logic of service
        Auditoria? auditoria;

        //Parametros de configuracion del servidor
        string _userBroker = ConfigurationManager.AppSettings["UserBroker"] ?? "";
        string _passBroker = ConfigurationManager.AppSettings["PassBroker"] ?? "";
        //Parametros de configuracion del servido


        try
        {
            //Configuration the server
            var optionsBuilder = new MqttServerOptionsBuilder()
            .WithDefaultEndpoint()
            .WithDefaultEndpointPort(1883)
            .Build();

            //Creation the server
            _serverM = new MqttFactory().CreateMqttServer(optionsBuilder);

            //Subscribe to the event to validate username and password
            _serverM.ValidatingConnectionAsync += e =>
                    {
                        //TODO: crear un metodo que guarda las conecciones al broker
                        Console.WriteLine($"Intento de conexión de: {e.ClientId}");
                        if (e.UserName != _userBroker || e.Password != _passBroker)
                        {
                            e.ReasonCode = MqttConnectReasonCode.BadUserNameOrPassword;
                        }

                        return Task.CompletedTask;
                    };

            //Procedure of message client
            _serverM.InterceptingPublishAsync += async e =>
            {
                RequestClient solicitud = new(Encoding.UTF8.GetString(e.ApplicationMessage.PayloadSegment), e.ClientId, e.ApplicationMessage.Topic);
                _service.Request = solicitud;
                auditoria = new(solicitud, _service.ProcedureRequest());

                Task task = auditoria.SalveRegistration();
            };


            //Begin the server MQTT
            await _serverM.StartAsync();

            Console.WriteLine("Servidor MQTT iniciado con autenticación.");
            Console.WriteLine("Presiona cualquier tecla para salir...");
            Console.ReadKey();

            // Detener el servidor MQTT
            await _serverM.StopAsync();
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error al iniciar el servidor MQTT: {ex.Message}");
        }
    }

    #endregion
}

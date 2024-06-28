using Demo_Broker_MQTT.Logic;
using Demo_Broker_MQTT.Modelos;
using MQTTnet.Protocol;
using MQTTnet.Server;
using System.Text;
using System.Threading.Tasks;
using System.Threading;

internal class ServerMQTT
{
    #region Metodo de creacion del servidor

    ///<summary>
    ///Creation and management the server or broker MQTT
    ///</summary>
    public async static Task BuilderBroker()
    {
        MqttServer _serverM;//-------------------------------------------------------------------------------------->Server Instruction
        Services _service = new();//------------------------------------------------------------------------------------------>Logic of service
        //Auditoria auditoria = new();

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
                Console.WriteLine("Se conecto: " + e.ClientId);
                //TODO: crear un metodo que guarda las conecciones al broker
                if (e.UserName != _userBroker || e.Password != _passBroker)
                {
                    e.ReasonCode = MqttConnectReasonCode.BadUserNameOrPassword;
                }
                else
                {
                    //Task task = auditoria.SalveConection(e.ClientId);
                }

                return Task.CompletedTask;
            };

            //Procedure of message client
            _serverM.InterceptingPublishAsync += e => {
                RequestClient solicitud = new(Encoding.UTF8.GetString(e.ApplicationMessage.PayloadSegment), e.ClientId, e.ApplicationMessage.Topic);
                _service.Request = solicitud;
                //auditoria = new(solicitud, _service.ProcedureRequest());

                //Pruebas
                Console.WriteLine("Topic: " + solicitud.Topic);
                Console.WriteLine("Mensaje: " + solicitud.Message);

               // Task task = auditoria.SalveRegistration();
                return Task.CompletedTask;
            };

            //Begin the server MQTT
            await _serverM.StartAsync();

            Console.WriteLine("Servidor MQTT iniciado con autenticación.");

            // Mantener el servidor en ejecución hasta que se reciba una señal de cancelación
            var completionSource = new TaskCompletionSource<bool>();
            Console.CancelKeyPress += (s, e) =>
            {
                e.Cancel = true;
                completionSource.SetResult(true);
            };
            await completionSource.Task;

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
using Demo_Broker_MQTT.DabaBase;

namespace Demo_Broker_MQTT.Modelos;

internal class Auditoria(RequestClient requestCient, uint resul) : SqlServer
{

    public async Task SalveRegistration()
    {
        await SetSalvarRequesAsync(requestCient, resul);
    }
}

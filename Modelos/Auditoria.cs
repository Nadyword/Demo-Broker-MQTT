using Demo_Broker_MQTT.DabaBase;

namespace Demo_Broker_MQTT.Modelos;

internal class Auditoria : SqlServer
{
    private RequestClient RequestCient { get; set; }
    private uint Resul { get; set; }

    public Auditoria()
    {

    }
    public Auditoria(RequestClient requestCient, uint resul)
    {
        RequestCient = requestCient;
        Resul = resul;
    }

    public async Task SalveRegistration() => await SetSalvarRequesAsync(RequestCient, Resul);

    public async Task SalveConection(string idClient) => await SetConexionesAsync(idClient);
}

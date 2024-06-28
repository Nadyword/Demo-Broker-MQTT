using Demo_Broker_MQTT.Modelos;

namespace Demo_Broker_MQTT.Logic;

internal class Services()
{
    public RequestClient? Request { get; set; }


    public uint ProcedureRequest()
    { 

        if (ValidateRequest())
        {
            Console.WriteLine("Se proceso la solicitud");
            return 200;
        }
        return 400;
    }

    private bool ValidateRequest()
    {
        if (Request == null) 
        {
            return false;
        }

        if (Request.Message != null && Request.Message != "" &&
            Request.Topic != null && Request.Topic != "" &&
            Request.IdClient != "" || Request.IdClient.Length > 4 || Request.IdClient != null)
        {
            return true;
        }
        return false;
    }
}

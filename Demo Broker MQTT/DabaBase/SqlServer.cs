using System.Data.SqlClient;
using System.Data;
using Demo_Broker_MQTT.Modelos;

namespace Demo_Broker_MQTT.DabaBase;

public class SqlServer
{
    private readonly string connectionString = ConfigurationManager.ConnectionStrings["MqttDbDemo"].ConnectionString;
    private string? _storedProcedureName;
    private Task ExecuteStoredProcedureAsync(SqlParameter[] parameters)
    {
        using SqlConnection connection = new(connectionString);
        using SqlCommand command = new(_storedProcedureName, connection);
        command.CommandType = CommandType.StoredProcedure;

        if (parameters != null)
        {
            command.Parameters.AddRange(parameters);
        }

        try
        {
            connection.Open();
            command.ExecuteNonQuery();
            connection.Close();
        }
        catch
        {

        }

        return Task.CompletedTask;
    }

    protected async Task SetSalvarRequesAsync(RequestClient request, uint result)
    {
        _storedProcedureName = "set_SalvarReques";
        SqlParameter[] parameters =
        [
                new SqlParameter("@Message", SqlDbType.NVarChar) { Value = request.Message },
                new SqlParameter("@IdClient", SqlDbType.NVarChar) { Value = request.IdClient },
                new SqlParameter("@Topic", SqlDbType.NVarChar) { Value = request.Topic },
                new SqlParameter("@Result", SqlDbType.Int) { Value = result }
            ];

        await ExecuteStoredProcedureAsync(parameters);

    }
}

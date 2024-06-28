/* =========================================
* Descrip:	Auditoria de las peticiones de los clientes
* Author :  SAMUEL SANCHEZ
* Cre.	 :	11-06-24
* ========================================= */ 
IF EXISTS (SELECT * FROM SYS.objects WHERE OBJECT_ID = OBJECT_ID(N'set_SalvarReques') AND TYPE IN (N'P', N'PC'))

BEGIN

	PRINT('El procedimiento  [dbo].[set_SalvarReques] existe y se borrar para ser creadi')
	DROP PROC [dbo].[set_SalvarReques]

END

	GO

CREATE PROCEDURE [dbo].[set_SalvarReques]
@Message				VARCHAR(300),
@IdClient			VARCHAR(100),
@Topic				VARCHAR(50),
@Result				INT

	AS

BEGIN

	INSERT INTO RequestClient ([Message], [IdClient], [Topic], [Result]) VALUES (@Message, @IdClient, @Topic,@Result)

END
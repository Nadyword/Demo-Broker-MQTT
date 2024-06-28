/* =========================================
* Descrip:	Guardar todos la conneciones al broker
* Author :  SAMUEL SANCHEZ
* Cre.	 :	11-06-24
* ========================================= */ 
IF EXISTS (SELECT * FROM SYS.objects WHERE OBJECT_ID = OBJECT_ID(N'set_Conexiones') AND TYPE IN (N'P', N'PC'))

BEGIN

	PRINT('El procedimiento  [dbo].[set_Conexiones] existe y se borrar para ser creadi')
	DROP PROC [dbo].[set_Conexiones]

END

	GO

CREATE PROCEDURE [dbo].[set_Conexiones]

@IdClient			VARCHAR(100)

	AS

BEGIN

	INSERT INTO Conexiones (idClient) VALUES (@IdClient)

END
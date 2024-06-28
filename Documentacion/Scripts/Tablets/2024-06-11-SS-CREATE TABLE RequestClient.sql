/* ========================================= 
* Descrip: Creacion de la table que registra todo las solicitudes al servidor
* Author : SAMUEL SANCHEZ
* Cre.	 : 10-06-24
* ========================================= */ 
IF NOT EXISTS (SELECT * FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_NAME = 'RequestClient')
BEGIN

	CREATE TABLE RequestClient(
	[Id]			INT IDENTITY(1,1),
	[Message]		VARCHAR(300),
	[IdClient]		VARCHAR(100),
	[Topic]			VARCHAR(50),
	[Result]		INT,
	[DateRequest]	DATETIME DEFAULT  GETDATE())

	PRINT('La tabla fue creada')

END
ELSE
BEGIN

	PRINT('La tabla ya existe')

END


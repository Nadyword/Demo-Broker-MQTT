/* ========================================= 
* Descrip: Creacion de la table que tiene la definicion de los codigos
* Author : SAMUEL SANCHEZ
* Cre.	 : 11-06-24
* ========================================= */ 
IF NOT EXISTS (SELECT * FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_NAME = 'Codigos')
BEGIN

CREATE TABLE Codigos (
	Codigo		INT IDENTITY,
	Descrip		VARCHAR(80))

	PRINT('La tabla fue creada')

END
ELSE
BEGIN

	PRINT('La tabla ya existe')

END

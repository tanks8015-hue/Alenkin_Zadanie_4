CREATE PROCEDURE sp_CreateOrder
    @UserID INT,
    @PartID INT,
    @Quantity INT,
    @WarehouseID INT
AS
BEGIN
    SET NOCOUNT ON;
    BEGIN TRANSACTION;
    BEGIN TRY
        DECLARE @NewOrderID INT;
        INSERT INTO Orders (UserID, Status) VALUES (@UserID, 'Processing');
        SET @NewOrderID = SCOPE_IDENTITY();

        DECLARE @CurrentPrice DECIMAL(10,2);
        SELECT @CurrentPrice = Price FROM Parts WHERE PartID = @PartID;
        
        INSERT INTO OrderDetails (OrderID, PartID, Quantity, UnitPrice) 
        VALUES (@NewOrderID, @PartID, @Quantity, @CurrentPrice);

        UPDATE Inventory 
        SET Quantity = Quantity - @Quantity 
        WHERE WarehouseID = @WarehouseID AND PartID = @PartID;

        IF (SELECT Quantity FROM Inventory WHERE WarehouseID = @WarehouseID AND PartID = @PartID) < 0
        BEGIN
            THROW 50001, 'Недостаточно товара на складе!', 1;
        END

        COMMIT TRANSACTION;
    END TRY
    BEGIN CATCH
        ROLLBACK TRANSACTION;
        THROW; 
    END CATCH
END;
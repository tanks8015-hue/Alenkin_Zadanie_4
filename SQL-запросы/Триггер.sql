CREATE TRIGGER trg_AuditPartPriceUpdate
ON Parts
AFTER UPDATE
AS
BEGIN
    IF UPDATE(Price)
    BEGIN
        INSERT INTO AuditLog (TableName, RecordID, Action, OldValue, NewValue, UserID)
        SELECT 
            'Parts',
            i.PartID,
            'UPDATE Price',
            CAST(d.Price AS NVARCHAR(50)), 
            CAST(i.Price AS NVARCHAR(50)), 
            NULL
        FROM inserted i
        JOIN deleted d ON i.PartID = d.PartID
        WHERE i.Price <> d.Price;
    END
END;
INSERT INTO Roles (RoleName) VALUES ('Admin'), ('Manager'), ('Worker');

INSERT INTO Users (Username, PasswordHash, RoleID) 
VALUES ('Ivan_Admin', 'hash123', 1), ('Anna_Manager', 'hash456', 2);

INSERT INTO Categories (CategoryName) VALUES ('Электроника'), ('Механика'), ('Пластик');

INSERT INTO Suppliers (SupplierName, ContactEmail) 
VALUES ('TechProm', 'sales@techprom.com'), ('MetalWorks', 'info@metalworks.com');

INSERT INTO Parts (PartName, CategoryID, SupplierID, Price) 
VALUES ('Микроконтроллер', 1, 1, 1500.00), ('Вал стальной', 2, 2, 850.50), ('Корпус', 3, 1, 300.00);

INSERT INTO Warehouses (Location) VALUES ('Склад №1 (Северный)'), ('Склад №2 (Южный)');

INSERT INTO Inventory (WarehouseID, PartID, Quantity) 
VALUES (1, 1, 100), (1, 2, 50), (2, 3, 200);
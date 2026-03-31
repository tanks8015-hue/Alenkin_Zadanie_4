-- Создание таблицы сотрудников (3-я нормальная форма)
CREATE TABLE Employees (
    EmpId INT IDENTITY(1,1) PRIMARY KEY, -- Первичный ключ [cite: 1, 10]
    FullName NVARCHAR(100) NOT NULL,
    Email NVARCHAR(100) NOT NULL,
    Salary DECIMAL(18, 2) NOT NULL,
    HireDate DATE DEFAULT GETDATE(),
    DeptId INT NOT NULL,
    CONSTRAINT FK_Employees_Departments FOREIGN KEY (DeptId) 
    REFERENCES Departments(DeptId) ON DELETE NO ACTION
);
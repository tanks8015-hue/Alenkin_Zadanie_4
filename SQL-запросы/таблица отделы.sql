CREATE TABLE Departments (
    DeptId INT IDENTITY(1,1) PRIMARY KEY,
    DeptName NVARCHAR(100) NOT NULL
);

INSERT INTO Departments (DeptName) VALUES ('Отдел разработки'), ('Бухгалтерия'), ('Отдел кадров');

CREATE TABLE Employees (
    EmpId INT IDENTITY(1,1) PRIMARY KEY,
    FullName NVARCHAR(100) NOT NULL,
    Email NVARCHAR(100) NOT NULL,
    Salary DECIMAL(18, 2) NOT NULL,
    HireDate DATE DEFAULT GETDATE(),
    DeptId INT NOT NULL,
    CONSTRAINT FK_Employees_Departments FOREIGN KEY (DeptId) REFERENCES Departments(DeptId)
);
CREATE TABLE IF NOT EXISTS atleta_telemetria (
    id BIGSERIAL PRIMARY KEY,
    atleta_id VARCHAR(50) NOT NULL,
    timestamp TIMESTAMPTZ DEFAULT NOW(),
    acel_x FLOAT NOT NULL,
    acel_y FLOAT NOT NULL,
    acel_z FLOAT NOT NULL,
    giro_x FLOAT NOT NULL,
    giro_y FLOAT NOT NULL,
    giro_z FLOAT NOT NULL
);

-- Tabela consolidada para métricas processadas pelo Python
CREATE TABLE IF NOT EXISTS atleta_metricas (
    id BIGSERIAL PRIMARY KEY,
    atleta_id VARCHAR(50) NOT NULL,
    timestamp TIMESTAMPTZ DEFAULT NOW(),
    distancia_km FLOAT NOT NULL,
    velocidade_kmh FLOAT NOT NULL,
    saltos INT NOT NULL,
    mudancas_direcao INT NOT NULL,
    zona_esforco VARCHAR(20) NOT NULL
);
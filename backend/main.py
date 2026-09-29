from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
import numpy as np
import math
from supabase_client import supabase

app = FastAPI(title="Smart Sports API - SESI LifeTech")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

class TelemetriaData(BaseModel):
    atleta_id: str
    acel_x: float
    acel_y: float
    acel_z: float
    giro_x: float
    giro_y: float
    giro_z: float

# Variáveis globais em memória para processamento contínuo
estado_atleta = {
    "ATLETA_07": {
        "distancia_m": 0.0,
        "saltos": 0,
        "mudancas_direcao": 0,
        "ultima_acel_mag": 9.8,
        "ultima_giro_z": 0.0
    }
}

@app.post("/api/telemetria")
async def receber_telemetria(data: TelemetriaData):
    atleta_id = data.atleta_id
    if atleta_id not in estado_atleta:
        estado_atleta[atleta_id] = {"distancia_m": 0.0, "saltos": 0, "mudancas_direcao": 0, "ultima_acel_mag": 9.8, "ultima_giro_z": 0.0}

    st = estado_atleta[atleta_id]

    # 1. Magnitude da Aceleração
    acel_mag = math.sqrt(data.acel_x**2 + data.acel_y**2 + data.acel_z**2)
    
    # 2. Detecção de Salto (pico de aceleração > 18 m/s²)
    if acel_mag > 18.0 and st["ultima_acel_mag"] <= 18.0:
        st["saltos"] += 1

    # 3. Detecção de Mudança de Direção (variação súbita no giroscópio)
    if abs(data.giro_z - st["ultima_giro_z"]) > 2.5:
        st["mudancas_direcao"] += 1

    # 4. Estimativa de Velocidade Instantânea e Distância (Aproximação em m/s)
    vel_ms = max(0.0, (acel_mag - 9.8) * 0.2) if acel_mag > 11.0 else (2.0 if acel_mag > 9.0 else 0.5)
    vel_kmh = vel_ms * 3.6
    st["distancia_m"] += vel_ms * 0.2
    distancia_km = round(st["distancia_m"] / 1000.0, 2)

    # 5. Classificação da Zona de Esforço
    if vel_kmh < 6.0:
        zona = "Baixa"
    elif vel_kmh < 15.0:
        zona = "Moderada"
    else:
        zona = "Alta"

    st["ultima_acel_mag"] = acel_mag
    st["ultima_giro_z"] = data.giro_z

    # Salvar telemetria bruta
    supabase.table("atleta_telemetria").insert({
        "atleta_id": atleta_id,
        "acel_x": data.acel_x, "acel_y": data.acel_y, "acel_z": data.acel_z,
        "giro_x": data.giro_x, "giro_y": data.giro_y, "giro_z": data.giro_z
    }).execute()

    # Salvar métricas processadas
    metricas = {
        "atleta_id": atleta_id,
        "distancia_km": distancia_km,
        "velocidade_kmh": round(vel_kmh, 1),
        "saltos": st["saltos"],
        "mudancas_direcao": st["mudancas_direcao"],
        "zona_esforco": zona
    }
    supabase.table("atleta_metricas").insert(metricas).execute()

    return {"status": "ok", "metricas": metricas}

@app.get("/api/dashboard/{atleta_id}")
async def obter_dashboard(atleta_id: str):
    res = supabase.table("atleta_metricas")\
        .select("*")\
        .eq("atleta_id", atleta_id)\
        .order("id", desc=True)\
        .limit(1)\
        .execute()
    
    if not res.data:
        raise HTTPException(status_code=404, detail="Atleta não encontrado")
    
    return res.data[0]
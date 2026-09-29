import os
from supabase import create_client, Client

SUPABASE_URL = "https://gvsziytndallswpbnhz.supabase.co"
SUPABASE_KEY = "sb_publishable_W38Ny10gKNTBeuTjk_UDGw_5x_R_ZWN"

supabase: Client = create_client(SUPABASE_URL, SUPABASE_KEY)
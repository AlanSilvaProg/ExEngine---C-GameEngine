# Metrics CLI

CLI para exibir métricas do projeto ExEngine.

## Compilação

```bash
cmake -S ExEngine-Source/EX_CLI/Metrics -B ExEngine-Source/EX_CLI/Metrics/build
cmake --build ExEngine-Source/EX_CLI/Metrics/build
```

## Uso

### Métricas completas (incluindo ThirdParty):
```bash
./ExEngine-Source/EX_CLI/Metrics/build/Metrics_CLI
```

### Métricas sem ThirdParty:
```bash
./ExEngine-Source/EX_CLI/Metrics/build/Metrics_CLI --no-thirdparty
```

## Informações Exibidas

- Quantidade de arquivos .h, .cpp e .hpp
- Total de arquivos de código
- Total de linhas de código
- Distribuição por diretórios (Engine, Editor, ThirdParty)

#include "SolanaUtils.h"
#include "IoTxChain-lib.h"

// Function implementations

void getSolBalance() {
  Serial.println("\n=== 🔹 getSolBalance() ===");

  const String wallet = PUBLIC_KEY_BASE58;
  uint64_t lamports = 0;

  if (solana.getSolBalance(wallet, lamports)) {
    Serial.print("Wallet address: ");
    Serial.println(wallet);
    Serial.print("SOL Balance (lamports): ");
    Serial.println(lamports);
    Serial.print("SOL Balance (SOL): ");
    Serial.println((float)lamports / 1e9, 9);
  } else {
    Serial.println("❌ Failed to fetch SOL balance.");
  }
}

void confirmTransaction(const String signature) {
  Serial.println("\n=== 🔹 confirmTransaction() ===");

  if (solana.confirmTransaction(signature, 5000)) {
      Serial.println("✅ Transaction confirmed!");
  } else {
    Serial.println("❌ Failed to confirm transaction.");
  }
}

void calculateDiscriminator(const String function_name) {
  Serial.println("\n=== 🔹 calculateDiscriminator() ===");

  std::vector<uint8_t> discriminator = solana.calculateDiscriminator(function_name.c_str());
  Serial.print("Function Name: ");
  Serial.println(function_name);
  Serial.print("Discriminator: ");
  
  Serial.print("Discriminator for '" + function_name + "': ");
    for (uint8_t b : discriminator) {
        if (b < 16) Serial.print("0");
        Serial.print(b, HEX);
    }
    Serial.println();
}

void sendAnchorInstructionWithPDA(
  const String& functionName,
  const std::vector<std::vector<uint8_t>>& customSeeds,
  const std::vector<uint8_t>& payload
) {
  Serial.println("\n=== 🔹 Anchor Instruction with PDA (Generic) ===");

  uint8_t privateKey[128] = "YOUR_PRIVATE_KEY_64_BYTE_BASE58"; // 64-byte base58 decoded

  Pubkey authority = Pubkey::fromBase58(PUBLIC_KEY_BASE58);
  Keypair signer = Keypair::fromPrivateKey(privateKey);

  std::vector<uint8_t> programId = base58ToPubkey(PROGRAM_ID);
  std::vector<uint8_t> tokenProgramId = base58ToPubkey(TOKEN_PROGRAM_ID); 

  String ata;
  solana.findAssociatedTokenAccount(PUBLIC_KEY_BASE58, MINT, ata);
  Pubkey pda = Pubkey::fromBase58(ata);

  Pubkey tempPda = Pubkey::fromBase58(createPDAFromSeeds(customSeeds));
  

  std::vector<uint8_t> discriminator = solana.calculateDiscriminator(functionName.c_str());
  std::vector<uint8_t> data = discriminator;
  data.insert(data.end(), payload.begin(), payload.end());

  Instruction ix(
      Pubkey{programId},
      functionName == "transfer_to_vault" ? std::vector<AccountMeta>{
          AccountMeta::writable(authority, true),
          AccountMeta::writable(pda, false),
          AccountMeta::writable(Pubkey::fromBase58(VAULT), false),
          AccountMeta{Pubkey::fromBase58(MINT), false, false},
          AccountMeta{Pubkey::fromBase58(TOKEN_PROGRAM_ID), false, false},
      } : std::vector<AccountMeta>{
        AccountMeta::writable(authority, true),
        AccountMeta::writable(tempPda, false),
        AccountMeta{Pubkey::fromBase58("11111111111111111111111111111111"), false, false}
      },
      data
  );

  Transaction tx;
  tx.fee_payer = authority;
  tx.recent_blockhash = solana.getLatestBlockhash();
  if (tx.recent_blockhash.isEmpty()) {
      Serial.println("❌ Failed to get blockhash!");
      return;
  }
  tx.add(ix);
  tx.sign({signer});
  String txBase64 = tx.serializeBase64();

  String txSig;
  if (solana.sendRawTransaction(txBase64, txSig)) {
      Serial.println("✅ Anchor tx sent! Signature: " + txSig);
      confirmTransaction(txSig);
  } else {
      Serial.println("❌ Anchor tx failed.");
  }
}

String createPDAFromSeeds(const std::vector<std::vector<uint8_t>>& customSeeds) {
  Serial.println("\n=== 🔹 Creating PDA from Seeds ===");
  
  // Program ID for the PDA derivation
  std::vector<uint8_t> programId = base58ToPubkey(PROGRAM_ID);
  
  // Output variables
  std::vector<uint8_t> pdaBytes;
  uint8_t bump;
  
  // Generate the PDA
  if (solana.findProgramAddress(customSeeds, programId, pdaBytes, bump)) {
    Serial.println("✅ PDA created successfully!");
    
    // Convert PDA bytes back to base58 string for display
    // Note: You'd need a proper bytes-to-base58 encoder here
    Serial.print("PDA Bump: ");
    Serial.println(bump);
    Serial.print("PDA Bytes (hex): ");
    for (uint8_t b : pdaBytes) {
      if (b < 16) Serial.print("0");
      Serial.print(b, HEX);
    }
    Serial.println();
    
    // Convert PDA bytes to base58 string for return
    Serial.print("PDA base 58: ");
    Serial.println(base58Encode(pdaBytes.data(), pdaBytes.size()));
    return base58Encode(pdaBytes.data(), pdaBytes.size());
  } 
  else {
    Serial.println("❌ Failed to create PDA");
    return "";
  }
} 
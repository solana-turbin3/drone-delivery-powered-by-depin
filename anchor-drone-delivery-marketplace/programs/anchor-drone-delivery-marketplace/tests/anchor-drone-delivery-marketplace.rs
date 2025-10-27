#[cfg(test)]

mod tests {
  use {
    anchor_lang::InstructionData,
    mollusk_svm::{program, Mollusk},
    solana_sdk::{
      account::Account, 
      instruction::{AccountMeta, Instruction}, 
      native_token::LAMPORTS_PER_SOL,
      pubkey},
  };

    const PROGRAM_ID: pubkey::Pubkey = pubkey!("9zVb1etgUuQp2PSTgYUAEC3dCrqM9rJVAQxkg6uXnsTF");
    const ADMIN: pubkey::Pubkey = pubkey::Pubkey::new_from_array([0x01; 32]);

    #[test]
    fn test_initialize() {
      //mollusk instance
      let mollusk = Mollusk::new(&PROGRAM_ID, "../../target/deploy/anchor_drone_delivery_marketplace");
     
      //pubkeys
      let marketplace_name = "Drone Delivery Marketplace";
      let (marketplace_pda, _marketplace_bump) = pubkey::Pubkey::find_program_address(
        &[b"marketplace", marketplace_name.as_bytes()], &PROGRAM_ID);

      let (system_program, system_account) = program::keyed_account_for_system_program();

      // build the accounts
      let admin_account = Account::new(1*LAMPORTS_PER_SOL, 0, &system_program);
      let marketplace_account = Account::new(0, 0, &system_program);
      
      //get the accounts meta
let ix_accounts: Vec<AccountMeta> = vec![
  AccountMeta::new(ADMIN, true),
  AccountMeta::new(marketplace_pda, false),
  AccountMeta::new_readonly(system_program, false),
 ];

//data 
let fee = 1;
let data = (anchor_drone_delivery_marketplace::instruction::Initialize{
  name: marketplace_name.to_string(), 
  fee: fee,
    })
.data(); //different for pinocchio


  //inject data to the accounts

  //build IX
let instruction = Instruction::new_with_bytes(
  PROGRAM_ID,
  &data,
  ix_accounts,
);

//build the transaction

//get tx accounts
let tx_accounts = &vec![
  (ADMIN, admin_account.clone()),
  (marketplace_pda, marketplace_account.clone()),
  (system_program, system_account.clone()),
];



//process our instruction
// Execute the instruction and get the result.
let _result = mollusk.process_instruction(&instruction, &tx_accounts);

  }







}
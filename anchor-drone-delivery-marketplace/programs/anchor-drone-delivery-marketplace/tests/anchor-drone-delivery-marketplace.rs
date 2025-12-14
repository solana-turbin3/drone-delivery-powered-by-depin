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
    const BUYER: pubkey::Pubkey = pubkey::Pubkey::new_from_array([0x02; 32]);
    const SELLER: pubkey::Pubkey = pubkey::Pubkey::new_from_array([0x03; 32]);

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

  #[test]
    fn test_list_product() {

      //mollusk instance
      let mollusk = Mollusk::new(&PROGRAM_ID, "../../target/deploy/anchor_drone_delivery_marketplace");

      //pubkeys
      let product_name = "Cough medicine";
      let product_description = "Relieves cough symptoms";
      let product_price = 10;//USDT ot USDC
      let (product_pda, _product_bump) = pubkey::Pubkey::find_program_address(
        &[b"product", product_name.as_bytes()], &PROGRAM_ID);
      let (system_program, system_account) = program::keyed_account_for_system_program();
      
      //build the accounts
      let seller_account = Account::new(1*LAMPORTS_PER_SOL, 0, &system_program);
      let product_account = Account::new(0, 0, &system_program);
      
      //get the accounts meta
      let ix_accounts: Vec<AccountMeta> = vec![
        AccountMeta::new(SELLER, true),
        AccountMeta::new(product_pda, false),
        AccountMeta::new_readonly(system_program, false),
      ];

      //data 
      let data = (anchor_drone_delivery_marketplace::instruction::ListProduct{
        product_name: product_name.to_string(),
        product_description: product_description.to_string(),
        product_price: product_price,
      })
      .data(); //different for pinocchio

      //inject data to the accounts
      let instruction = Instruction::new_with_bytes(
        PROGRAM_ID,
        &data,
        ix_accounts,
      );

      //build the transaction
      let tx_accounts = &vec![
        (SELLER, seller_account.clone()),
        (product_pda, product_account.clone()),
        (system_program, system_account.clone()),
      ];

      //process our instruction
      let _result = mollusk.process_instruction(&instruction, &tx_accounts);

    }

    #[test]
    fn test_purchase_product() {

      //mollusk instance
      let mollusk = Mollusk::new(&PROGRAM_ID, "../../target/deploy/anchor_drone_delivery_marketplace");
      
      //pubkeys
      let product_name = "Cough medicine";
      let quantity = 1;
      let (product_pda, _product_bump) = pubkey::Pubkey::find_program_address(
        &[b"product", product_name.as_bytes()], &PROGRAM_ID);
      let (order_info_pda, _order_info_bump) = pubkey::Pubkey::find_program_address(
        &[b"order", BUYER.as_ref()], &PROGRAM_ID);
      let (system_program, system_account) = program::keyed_account_for_system_program();
      
      //build the accounts
      let buyer_account = Account::new(1*LAMPORTS_PER_SOL, 0, &system_program);
      let product_account = Account::new(0, 0, &system_program);
      let order_info_account = Account::new(0, 0, &system_program);
      
      //get the accounts meta
      let ix_accounts: Vec<AccountMeta> = vec![
        AccountMeta::new(BUYER, true),
        AccountMeta::new(product_pda, false),
        AccountMeta::new(order_info_pda, false),
        AccountMeta::new_readonly(system_program, false), 
      ];

      //data 
      let data = (anchor_drone_delivery_marketplace::instruction::PurchaseProduct{
        product_name: product_name.to_string(),
        quantity: quantity,
      })
      .data(); //different for pinocchio
      
      //inject data to the accounts
      let instruction = Instruction::new_with_bytes(
        PROGRAM_ID,
        &data,
        ix_accounts,
      );

      //build the transaction
      let tx_accounts = &vec![
        (BUYER, buyer_account.clone()),
        (product_pda, product_account.clone()),
        (order_info_pda, order_info_account.clone()),
        (system_program, system_account.clone()),
      ];

        //process our instruction
        let _result = mollusk.process_instruction(&instruction, &tx_accounts);

      }

}